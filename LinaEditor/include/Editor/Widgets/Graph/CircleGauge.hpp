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

#pragma once

#include "Core/GUI/Widgets/Widget.hpp"

namespace Lina
{
	class Text;
	class Circle;
}; // namespace Lina

namespace Lina::Editor
{
	class CircleGauge : public Widget
	{
	public:
		CircleGauge()		   = default;
		virtual ~CircleGauge() = default;

		struct Properties
		{
		};

		virtual void Construct() override;

		inline Properties& GetProps()
		{
			return m_props;
		}

		Circle* GetCircle() const
		{
			return m_circle;
		}

		Text* GetTitle() const
		{
			return m_title;
		}

		Text* GetDataText() const
		{
			return m_dataText;
		}

	private:
		Vector<LinaVG::Vec2> m_groupPoints;
		Properties			 m_props	= {};
		Circle*				 m_circle	= nullptr;
		Text*				 m_title	= nullptr;
		Text*				 m_dataText = nullptr;
	};

	LINA_WIDGET_BEGIN(CircleGauge, Graph)
	LINA_CLASS_END(CircleGauge)
} // namespace Lina::Editor
