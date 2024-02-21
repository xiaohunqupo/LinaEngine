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

#include "Core/GUI/Widgets/Primitives/Button.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include <LinaGX/Core/InputMappings.hpp>

namespace Lina
{
	void Button::Construct()
	{
		m_text = Allocate<Text>();
		AddChild(m_text);
	}

	void Button::Tick(float delta)
	{
		if (!m_isHovered)
			m_isPressed = false;

		if (m_props.widthFit == Fit::FromChildren)
			m_rect.size.x = m_text->GetSize().x + m_props.margins.left + m_props.margins.right;

		if (m_props.heightFit == Fit::FromChildren)
			m_rect.size.y = m_text->GetSize().y + m_props.margins.top + m_props.margins.bottom;

		const Vector2& textSize = m_text->GetSize();
		m_text->SetPos(Vector2(m_rect.pos.x + m_rect.size.x * 0.5f - textSize.x * 0.5f, m_rect.pos.y + m_rect.size.y * 0.5f - textSize.y * 0.5f));
	}

	void Button::Draw(int32 threadIndex)
	{
		LinaVG::StyleOptions style;
		style.rounding				   = m_props.rounding;
		style.outlineOptions.thickness = m_props.outlineThickness;
		style.outlineOptions.color	   = m_props.colorOutline.AsLVG4();

		if (m_isPressed)
			style.color = m_props.colorPressed.AsLVG4();
		else if (m_isHovered)
			style.color = m_props.colorHovered.AsLVG4();
		else
		{
			style.color.start		 = m_props.colorDefaultStart.AsLVG4();
			style.color.end			 = m_props.colorDefaultEnd.AsLVG4();
			style.color.gradientType = LinaVG::GradientType::Vertical;
		}

		LinaVG::DrawRect(threadIndex, m_rect.pos.AsLVG(), (m_rect.pos + m_rect.size).AsLVG(), style, 0.0f, m_drawOrder);

		m_text->Draw(threadIndex);
	}

	void Button::SetProps(const Properties& props)
	{
		m_text->SetProps({.text = props.text, .font = props.font});
		m_props = props;
	}

	bool Button::OnMouse(uint32 button, LinaGX::InputAction act)
	{
		if (button != LINAGX_MOUSE_0 || !m_isHovered)
		{
			return Widget::OnMouse(button, act);
		}

		if (act == LinaGX::InputAction::Pressed)
			m_isPressed = true;

		if (act == LinaGX::InputAction::Released)
		{
			m_isPressed = false;
			if (m_props.onClicked)
				m_props.onClicked();
		}

		return true;
	}
} // namespace Lina
