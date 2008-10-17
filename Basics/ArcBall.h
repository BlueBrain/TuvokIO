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
  \file    ArcBall.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
           Based on the NeHe Tutorial 48
  \date    October 2008
*/

#pragma once

#include <vector>
#include <Basics/Vectors.h>

/** \class ArcBall */
class ArcBall
{
public:
	ArcBall(unsigned int iWinWidth=0, unsigned int iWinHeight=0);

  void SetWindowSize(unsigned int iWinWidth, unsigned int iWinHeight);
  void Click(UINTVECTOR2 vPosition);
  FLOATQUATERNION4 Drag(UINTVECTOR2 vPosition);

protected:
  static float ms_fEpsilon;

  FLOATVECTOR3 m_vStartDrag;   ///< Saved click vector
  unsigned int m_iWinWidth;    ///< Mouse bounds width
  unsigned int m_iWinHeight;   ///< Mouse bounds height

  FLOATVECTOR3 MapToSphere(UINTVECTOR2 vPosition) const;
};