/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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
  \file    TransferFunction1D.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    September 2008
*/

#pragma once

#ifndef TRANSFERFUNCTION1D
#define TRANSFERFUNCTION1D

#include <string>
#include <vector>
#include <Basics/Vectors.h>
#include <Basics/Grids.h>

typedef Grid1D<unsigned int> Histogram1D;
typedef Grid1D<float> NormalizedHistogram1D;

class TransferFunction1D
{
public:
  TransferFunction1D(size_t iSize = 0);
  TransferFunction1D(const std::string& filename);
  ~TransferFunction1D(void);
  
  void SetDefault();

  void Resize(size_t iSize);
  void Resample(size_t iTargetSize);

  bool Load(const std::string& filename);
  bool Load(const std::string& filename, size_t iTargetSize);
  bool Load(std::ifstream& file);
  bool Load(std::ifstream& file, size_t iTargetSize);
  bool Save(const std::string& filename);
  bool Save(std::ofstream& file);

  void Clear();

  void GetByteArray(unsigned char** pcData, unsigned char cUsedRange=255);
  void GetShortArray(unsigned short** psData, unsigned short sUsedRange=4095);
  void GetFloatArray(float** pfData);

  std::vector< FLOATVECTOR4 > vColorData;

};

#endif // TRANSFERFUNCTION1D
