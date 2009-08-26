/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    Quantize.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Quantization routines.
*/

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <functional>
#include <limits>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/cstdint.hpp>
#include "IOManager.h"  // for the size defines
#include "Basics/LargeRAWFile.h"

namespace {
/// "Compile time type info"
/// is_signed: whether or not this type is signed.
///            Note that this exists in std::numeric_limits as well, but was
///            incorrectly specified so that it is not a compile-time constant.
/// size_type: unsigned variant of 'T'.
///@{
template <typename T> struct ctti { };
template<> struct ctti<short>  { enum { is_signed = 1 };
                                 typedef unsigned short size_type; };
template<> struct ctti<bool>   { enum { is_signed = 0 };
                                 typedef bool size_type; };
template<> struct ctti<int>    { enum { is_signed = 1 };
                                 typedef unsigned int size_type; };
template<> struct ctti<long>   { enum { is_signed = 1 };
                                 typedef unsigned long size_type; };
template<> struct ctti<float>  { enum { is_signed = 1 };
                                 typedef float size_type; };
template<> struct ctti<double> { enum { is_signed = 1 };
                                 typedef double size_type; };
template<> struct ctti<unsigned int>   { enum { is_signed = 0 };
                                         typedef unsigned int size_type; };
template<> struct ctti<unsigned char>  { enum { is_signed = 0 };
                                         typedef unsigned char size_type; };
template<> struct ctti<unsigned short> { enum { is_signed = 0 };
                                         typedef unsigned short size_type; };
///@}
};

/// Progress policies.  Must implement a constructor and `notify' method which
/// both take a `T'.  The constructor is given the max value; the notify method
/// is given the current value.
///    NullProgress -- nothing, use when you don't care.
///    TuvokProgress -- forward progress info to Tuvok debug logs.
///@{
template <typename T>
struct NullProgress {
  NullProgress(T) {};
  static void notify(T) {}
};
template <typename T>
struct TuvokProgress {
  TuvokProgress(T total) : tMax(total) {}
  void notify(T current) const {
    MESSAGE("Computing value range (%5.3f%% complete).",
            static_cast<double>(current) / static_cast<double>(tMax)*100.0);
  }
  private: T tMax;
};
///@}

/// Data source policies.  Must implement:
///   constructor: takes an opened file.
///   size(): returns the number of elements in the file.
///   read(data, b): reads `b' bytes in to `data'.  Returns number of elems
///                  actually read.
/// ios_data_src -- data source for C++ iostreams.
/// raw_data_src -- data source for Basics' LargeRAWFile.
///@{
template <typename T>
struct ios_data_src {
  ios_data_src(std::ifstream& fs) : ifs(fs) {
    if(!ifs.is_open()) {
      throw std::runtime_error(__FILE__);
    }
  }

  boost::uint64_t size() {
    std::streampos cur = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    boost::uint64_t retval = ifs.tellg();
    ifs.seekg(cur, std::ios::beg);
    return retval/sizeof(T);
  }
  size_t read(unsigned char *data, size_t max_bytes) {
    ifs.read(reinterpret_cast<char*>(data), std::streamsize(max_bytes));
    return ifs.gcount()/sizeof(T);
  }
  private:
    std::ifstream& ifs;
    const char *filename;
};

template <typename T>
struct raw_data_src {
  raw_data_src(LargeRAWFile& r) : raw(r) {
    if(!raw.IsOpen()) {
      throw std::runtime_error(__FILE__);
    }
  }

  boost::uint64_t size() { return raw.GetCurrentSize() / sizeof(T); }
  size_t read(unsigned char *data, size_t max_bytes) {
    return raw.ReadRAW(data, max_bytes)/sizeof(T);
  }
  private: LargeRAWFile& raw;
};
///@}

/// If we just do a standard "is the value of type T <= 4096?" then compilers
/// complain with 8bit types because the comparison is always true.  Great,
/// thanks, I didn't know that.  So, this basically gets around a warning.
/// I love C++ sometimes.
namespace { namespace Fits {
  template<typename T> bool in12bits(T v) { return v <= 4096; }
  template<> bool in12bits(unsigned char) { return true; }
}; };

/// Histogram policies.  minmax can sometimes compute a 1D histogram as it
/// marches over the data.  It may happen that the data must be quantized
/// though, forcing the histogram to be recalculated.
/// Must implement:
///    bin(T): bin the given value.  return false if we shouldn't bother
///            computing the histogram anymore.
template<typename T> struct NullHistogram {
  static bool bin(T) { return false; }
};
// Calculate a 12Bit histo, but when we encounter a value which does not fit
// (i.e., we know we'll need to quantize), don't bother anymore.
template<typename T> struct Unsigned12BitHistogram {
  Unsigned12BitHistogram(std::vector<UINT64>& h)
    : histo(h), calculate(true) {}

  bool bin(T value) {
    if(!calculate || !Fits::in12bits<T>(value)) {
      calculate = false;
    } else {
      update(value);
    }
    return calculate;
  }

  void update(T value) {
    // Calculate our bias factor up front.
    typename ctti<T>::size_type bias;
    bias = static_cast<typename ctti<T>::size_type>
                      (std::fabs(static_cast<double>
                                 (std::numeric_limits<T>::min())));

    if(static_cast<typename ctti<T>::size_type>(value) < histo.size()) {
      typename ctti<T>::size_type u_value;
      u_value = ctti<T>::is_signed ? value + bias : value;
      if(u_value < histo.size()) {
        ++histo[static_cast<size_t>(u_value)];
      }
    }
  }
  private:
    std::vector<UINT64>& histo;
    bool calculate;
};

/// Computes the minimum and maximum of a conceptually one dimensional dataset.
/// Takes policies tell it how to access data && notify external entities of
/// progress.
/// @todo shouldn't hardcode INCORESIZE in here.
template <typename T,
          template <typename T> class DataSrc,
          template <typename T> class Histogram,
          class Progress>
std::pair<T,T> io_minmax(DataSrc<T> ds, Histogram<T> histogram,
                         const Progress& progress)
{
  std::vector<T> data(INCORESIZE);
  boost::uint64_t iPos = 0;
  boost::uint64_t iSize = ds.size();

  // Default min is the max value representable by the data type.  Default max
  // is the smallest value representable by the data type.
  std::pair<T,T> t_minmax(std::numeric_limits<T>::max(),
                          -(std::numeric_limits<T>::max()-1));
  // ... but if the data type is unsigned, the correct default 'max' is 0.
  if(!ctti<T>::is_signed) {
    t_minmax.second = std::numeric_limits<T>::min(); // ... == 0.
  }

  while(iPos < iSize) {
    size_t n_records = ds.read((unsigned char*)(&(data.at(0))), INCORESIZE);
    data.resize(n_records);
    if(n_records == 0) { break; } // bail out if the read gave us nothing.

    typedef typename std::vector<T>::const_iterator iterator;
    std::pair<iterator,iterator> cur_mm = boost::minmax_element(data.begin(),
                                                                data.end());
    t_minmax.first = std::min(t_minmax.first, *cur_mm.first);
    t_minmax.second = std::max(t_minmax.second, *cur_mm.second);

    // Run over the data again and bin the data for the histogram.
    for(size_t i=0; i < n_records && histogram.bin(data[i]); ++i) { }

    iPos += boost::uint64_t(n_records);

    progress.notify(iPos);
  }
  return t_minmax;
}