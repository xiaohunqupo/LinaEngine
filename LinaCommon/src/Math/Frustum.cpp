/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Math/Frustum.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/AABB.hpp"
#include "Log/Log.hpp"

namespace Lina
{
    void Frustum::Calculate(const Matrix& matrix, bool normalize)
    {
        Matrix  m      = matrix.Transpose();
        Vector4 vleft   = m[3] + m[0];
        Vector4 vright  = m[3] - m[0];
        Vector4 vtop    = m[3] - m[1];
        Vector4 vbottom = m[3] + m[1];
        Vector4 vnearP  = m[3] + m[2];
        Vector4 vfarP   = m[3] - m[2];

        m_left.normal   = vleft.XYZ();
        m_left.distance = vleft.w;

        m_right.normal   = vright.XYZ();
        m_right.distance = vright.w;

        m_top.normal   = vtop.XYZ();
        m_top.distance = vtop.w;

        m_bottom.normal   = vbottom.XYZ();
        m_bottom.distance = vbottom.w;

        m_near.normal   = vnearP.XYZ();
        m_near.distance = vnearP.w;

        m_far.normal   = vfarP.XYZ();
        m_far.distance = vfarP.w;

        if (normalize)
        {
            m_left.normal.Normalize();
            m_right.normal.Normalize();
            m_top.normal.Normalize();
            m_bottom.normal.Normalize();
            m_near.normal.Normalize();
            m_far.normal.Normalize();
        }
    }
    FrustumTest Frustum::TestIntersection(const AABB& aabb)
    {
        FrustumTest    test   = FrustumTest::Inside;
        Vector<Plane*> planes = {&m_left, &m_right, &m_top, &m_bottom, &m_near, &m_far};

        for (auto* p : planes)
        {
            const float   pos    = p->distance;
            const Vector3 normal = p->normal;

            if (normal.Dot(aabb.GetPositive(normal)) + pos < 0.0f)
                return FrustumTest::Outside;

            if (normal.Dot(aabb.GetNegative(normal)) + pos < 0.0f)
                test = FrustumTest::Intersects;
        }

        return test;
    }
} // namespace Lina