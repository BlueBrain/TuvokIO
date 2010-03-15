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
  \file    DSFactory.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Instantiates the correct kind of dataset from a file
*/
#pragma once
#ifndef TUVOK_DS_FACTORY_H
#define TUVOK_DS_FACTORY_H

#include "StdTuvokDefines.h"

#ifdef DETECTED_OS_WINDOWS
# include <memory>
#else
# include <tr1/memory>
#endif
#include <list>
#include <stdexcept>
#include "TuvokIOError.h"

// Get rid of the warning about "non-empty throw specification".
#ifdef DETECTED_OS_WINDOWS
  #pragma warning(disable:4290)
#endif

namespace tuvok {

class FileBackedDataset;

namespace io {

class DSFactory {
public:
  /// Instantiates a new dataset.
  /// @param the filename for the dataset to be opened
  /// @param maximum brick size allowed by the caller
  /// @param whether the dataset should do expensive work to verify that the
  ///        file is valid/correct.
  FileBackedDataset* Create(const std::string&, UINT64, bool) const
                     throw(DSOpenFailed);

  void AddReader(std::tr1::shared_ptr<FileBackedDataset>);

private:
  // We can't copy datasets.  So we store pointers to them instead.
  std::list<std::tr1::shared_ptr<FileBackedDataset> > datasets;
};

} // io
} // tuvok

#endif // TUVOK_DS_FACTORY_H