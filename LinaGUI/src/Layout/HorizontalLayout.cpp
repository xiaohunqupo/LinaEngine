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

#include "Layout/HorizontalLayout.hpp"

namespace Lina::GUI
{
    void HorizontalLayout::Draw()
    {
        DebugDraw();

        if (type == HorizontalLayoutType::EquallyDistribute)
        {
            const float childSize       = static_cast<float>(m_children.size());
            const float totalSpacedArea = spacing * (childSize + 1);
            const float xPerChild       = (size.x - totalSpacedArea) / childSize;
            const float halfXPerChild   = xPerChild * 0.5f;

            float lastX = 0.0f;

            for (auto& c : m_children)
            {
                c->position.y = position.y;
                c->position.x = lastX + spacing * xPerChild;
                lastX         = c->position.x;
                c->size.y     = size.y;
                c->size.x     = xPerChild;
                c->Draw();
            }
        }
        else if (type == HorizontalLayoutType::Append)
        {
            float lastEndX = 0.0f;
            for (auto& c : m_children)
            {
                c->Calculate();
                c->position.y = position.y;
                c->position.x = lastEndX + spacing + c->size.x * 0.5f;
                lastEndX      = c->position.x + c->size.x * 0.5f;
                c->Draw();
            }
        }
    }
} // namespace Lina::GUI
