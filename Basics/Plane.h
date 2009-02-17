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
  \file    Plane.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
*/

#pragma once

#ifndef TUVOK_PLANE_H 
#define TUVOK_PLANE_H

#include <cassert>
#include <vector>
#include "Vectors.h"

/// Encapsulates a plane with a perpendicular vector, the cross product of the
/// plane and the perpendicular vector form a basis.
class ExtendedPlane {
  public:
    ExtendedPlane(const PLANE<float>& p = PLANE<float>(0,0,1,0),
                  const FLOATVECTOR3& perp = FLOATVECTOR3(0,1,0));

    void Transform(const FLOATMATRIX4&);
    void TransformIT(const FLOATMATRIX4&);

    bool Quad(const FLOATVECTOR3& vEye, const FLOATVECTOR3& vDatasetCenter,
              std::vector<FLOATVECTOR3>& quad, const float fWidgetSize=0.5f);

    static const PLANE<float> ms_Plane;
    static const FLOATVECTOR3 ms_Perpendicular;

    float& d() { return m_Plane.w; }
    const float& d() const { return m_Plane.w; }

    float x() const { return m_Plane.x; }
    float y() const { return m_Plane.y; }
    float z() const { return m_Plane.z; }

    const PLANE<float>& Plane() const { return m_Plane; }

    void transform(const FLOATMATRIX4 &m) {
      m_Plane = m_Plane * m;
      m_Perpendicular = m_Perpendicular * m;
    }
    ExtendedPlane operator *(const FLOATMATRIX4 &m) const {
      return ExtendedPlane(m_Plane * m, m_Perpendicular * m);
    }

    bool operator ==(const ExtendedPlane &ep) const {
      return m_Plane == ep.m_Plane;
    }

  private:
    PLANE<float> m_Plane;
    FLOATVECTOR3 m_Perpendicular;
};

#endif // TUVOK_PLANE_H
