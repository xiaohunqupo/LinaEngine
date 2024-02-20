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

#include "Editor/Widgets/Testbed.hpp"
#include "Editor/CommonEditor.hpp"
#include "Common/System/System.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/Button.hpp"
#include "Core/GUI/Theme.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Resource/Font.hpp"

namespace Lina::Editor
{

	void Testbed::Construct()
	{

		auto* resMan	  = m_system->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		auto* iconFont	  = resMan->GetResource<Font>(ICON_FONT_SID);
		auto* defaultFont = resMan->GetResource<Font>(Theme::GetDef().defaultFont);

		// Icon
		{
			Text* text = Allocate<Text>();
			text->SetProps({
				.text  = "A",
				.font  = iconFont,
				.color = Color(1.0f, 1.0f, 1.0f, 1.0f),
			});
			text->SetPos(Vector2(10, 10));
			AddChild(text);
		}

		// Testbed Text
		{
			Text* text = Allocate<Text>();
			text->SetProps({
				.text  = "Testbed",
				.font  = defaultFont,
				.color = Color(1.0f, 1.0f, 1.0f, 1.0f),
			});
			text->SetPos(Vector2(30, 10));
			AddChild(text);
		}

		// Button
		{
			RectBackground bg = {
				.enabled	= true,
				.startColor = Theme::GetDef().background2,
				.endColor	= Theme::GetDef().background2,
			};

			RectBackground bgHovered = {
				.enabled	= true,
				.startColor = Theme::GetDef().background3,
				.endColor	= Theme::GetDef().background3,
			};

			Button* button = Allocate<Button>();
			button->SetProps({
				.widthFit		   = Fit::Fixed,
				.heightFit		   = Fit::Fixed,
				.margins		   = {},
				.font			   = defaultFont,
				.text			   = "Button",
				.background		   = bg,
				.backgroundHovered = bgHovered,
			});
			button->SetSize(Vector2(60, 20));
			button->SetPos(Vector2(10, 30));
			AddChild(button);
		}
	}

	void Testbed::Tick(float delta)
	{
		m_rect = m_parent->GetRect();
		Widget::Tick(delta);
	}

	void Testbed::Draw(int32 threadIndex)
	{
		RectBackground bgSettings = {
			.enabled	= true,
			.startColor = Theme::GetDef().background1,
			.endColor	= Theme::GetDef().background1,
		};

		WidgetUtility::DrawRectBackground(threadIndex, bgSettings, m_rect, m_drawOrder);
		Widget::Draw(threadIndex);
	}

} // namespace Lina::Editor
