﻿/*
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

#include "GUI/Nodes/Widgets/GUINodeTextArea.hpp"
#include "GUI/Utility/GUIUtility.hpp"
#include "Graphics/Interfaces/ISwapchain.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"
#include "Graphics/Interfaces/IWindow.hpp"
#include "GUI/Utility/GUIUtility.hpp"
#include "Core/Theme.hpp"
#include "Math/Math.hpp"
#include "Input/Core/InputMappings.hpp"
#include "Input/Core/Input.hpp"
#include "Core/SystemInfo.hpp"
#include "GUI/Drawers/GUIDrawerBase.hpp"
#include "Core/Editor.hpp"
#include "System/ISystem.hpp"
#include "FileSystem/FileSystem.hpp"
#include "Core/PlatformProcess.hpp"

namespace Lina::Editor
{
	GUINodeTextArea::GUINodeTextArea(GUIDrawerBase* drawer, int drawOrder) : GUINode(drawer, drawOrder)
	{
		m_input = m_drawer->GetEditor()->GetSystem()->CastSubsystem<Input>(SubsystemType::Input);
	}

	void GUINodeTextArea::Draw(int threadID)
	{
		if (!GetIsVisible())
			return;

		const float	  padding	   = Theme::GetProperty(ThemeProperty::GeneralItemPadding, m_window->GetDPIScale());
		const float	  widgetHeight = Theme::GetProperty(ThemeProperty::WidgetHeightTall, m_window->GetDPIScale());
		const Vector2 lastTextSize = GetStoreSize("TitleText"_hs, m_title);
		m_caretHeight			   = widgetHeight * 0.6f;

		// Adjust rect
		{
			SetWidgetHeight(ThemeProperty::WidgetHeightTall);
			if (!m_widthOverridenOutside)
				GUINode::ConsumeAvailableWidth();
		}

		GUIUtility::DrawWidgetBackground(threadID, m_rect, m_window->GetDPIScale(), m_drawOrder);
		LinaVG::TextOptions opts;
		opts.font		 = Theme::GetFont(FontType::DefaultEditor, m_window->GetDPIScale());
		m_initialTextPos = Vector2(m_rect.pos.x + padding, m_rect.pos.y + m_rect.size.y * 0.5f + lastTextSize.y * 0.5f);

		if (!m_isEditing)
		{
			LinaVG::DrawTextNormal(threadID, m_title.c_str(), LV2(m_initialTextPos), opts, 0.0f, m_drawOrder, true);
		}
		else
		{

			LinaVG::StyleOptions caretStyle;
			caretStyle.color		 = LV4(Theme::TC_Silent3);
			caretStyle.color.start.w = caretStyle.color.end.w = 0.5f;
			caretStyle.thickness							  = 1.2f * m_window->GetDPIScale();

			// One by one.
			const uint32 sz = static_cast<uint32>(m_characters.size());
			for (uint32 i = 0; i < sz; i++)
			{
				const auto&	  cd   = m_characters[i];
				const WString wstr = WString(1, cd.c);
				const String  str  = Internal::WideStringToString(wstr);
				const Vector2 pos  = m_initialTextPos + Vector2(cd.x, 0.0f);
				LinaVG::DrawTextNormal(threadID, str.c_str(), LV2(pos), opts, 0.0f, m_drawOrder + 1, true);
			}

			// Caret
			{
				// First timer
				{
					m_caretTimer += SystemInfo::GetDeltaTimeF();
					if (m_caretTimer > 0.5f)
					{
						m_caretTimer	  = 0.0f;
						m_shouldDrawCaret = !m_shouldDrawCaret;
					}
				}

				if (m_isDragging && !m_characters.empty())
				{
					m_caretIndexEnd = FindCaretIndexFromMouse();
				}

				if (m_shouldDrawCaret)
				{
					const uint32 caretIndex = m_caretIndexEnd;
					float		 caretX		= 0.0f;

					if (sz != 0)
					{
						if (caretIndex == sz)
						{
							const auto& cd = m_characters[sz - 1];
							caretX		   = cd.x + cd.sizeX;
						}
						else
							caretX = m_characters[caretIndex].x;
					}

					const Vector2 p1 = Vector2(m_initialTextPos.x + caretX, m_rect.pos.y + m_rect.size.y * 0.5f - m_caretHeight * 0.5f);
					const Vector2 p2 = Vector2(m_initialTextPos.x + caretX, m_rect.pos.y + m_rect.size.y * 0.5f + m_caretHeight * 0.5f);
					LinaVG::DrawLine(threadID, LV2(p1), LV2(p2), caretStyle, LinaVG::LineCapDirection::None, 0.0f, m_drawOrder + 1);
				}
			}

			// Background if multi-selected
			if (m_caretIndexStart != m_caretIndexEnd)
				DrawMultiSelection(threadID);
		}
	}

	void GUINodeTextArea::OnClicked(uint32 button)
	{
		if (button != LINA_MOUSE_0)
			return;

		if (!m_isEditing)
		{
			m_isEditing		  = true;
			m_caretIndexStart = 0;
			m_caretIndexEnd	  = static_cast<uint32>(m_characters.size());
		}
		else
		{
			if (m_characters.empty())
				return;

			// Find the correct caret position based on click
			uint32 caretIndex = 0;

			const float padding = Theme::GetProperty(ThemeProperty::GeneralItemPadding, m_window->GetDPIScale());

			const Vector2 mp = m_window->GetMousePosition();
			if (m_rect.IsPointInside(mp))
				caretIndex = FindCaretIndexFromMouse();

			m_caretIndexEnd = m_caretIndexStart = caretIndex;
		}
	}

	void GUINodeTextArea::OnClickedOutside(uint32 button)
	{
		FinishEditing();
	}

	void GUINodeTextArea::SetTitle(const String& title)
	{
		GUINode::SetTitle(title);
		m_characters.clear();

		WString wstr = Internal::StringToWString(title);

		const wchar_t* currChar = wstr.c_str();
		float		   x		= 0.0f;

		while (*currChar != '\0')
		{
			const WString  wstr		= WString(1, *currChar);
			const String   str		= Internal::WideStringToString(wstr);
			const StringID sid		= TO_SID(str);
			const Vector2  charSize = GetStoreSize(sid, str, FontType::DefaultEditor);
			CharData	   cd		= CharData{.c = *currChar, .x = x, .sizeX = charSize.x};
			m_characters.push_back(cd);
			x += charSize.x;
			currChar++;
		}
	}

	void GUINodeTextArea::OnDragBegin()
	{
		if (m_characters.empty())
			return;

		m_isEditing		  = true;
		m_caretIndexStart = FindCaretIndexFromMouse();
	}

	void GUINodeTextArea::OnDoubleClicked()
	{
		SelectAll();
	}

	void GUINodeTextArea::OnKey(uint32 key, InputAction act)
	{
		if (!m_isEditing || act == InputAction::Released)
			return;

		const uint32 sz = static_cast<uint32>(m_characters.size());

		if (key == LINA_KEY_RIGHT)
		{
			if (m_caretIndexEnd != m_caretIndexStart)
			{
				if (m_caretIndexEnd != sz)
					m_caretIndexEnd++;
			}
			else
			{
				if (m_caretIndexStart != sz)
					m_caretIndexStart = ++m_caretIndexEnd;
			}
		}
		else if (key == LINA_KEY_LEFT)
		{
			if (m_caretIndexEnd != m_caretIndexStart)
			{
				if (m_caretIndexEnd != 0)
					m_caretIndexEnd--;
			}
			else
			{
				if (m_caretIndexStart != 0)
					m_caretIndexStart = --m_caretIndexEnd;
			}
		}
		else if (key == LINA_KEY_DOWN || key == LINA_KEY_UP)
			m_caretIndexStart = m_caretIndexEnd;
		else
		{
			if (key == LINA_KEY_RETURN)
				FinishEditing();
			else if (key == LINA_KEY_BACKSPACE)
			{
				EraseChar();
			}
			else if (m_inputMask.IsSet(m_input->GetCharacterMask(key)))
			{
				wchar_t ch = m_input->GetCharacterFromKey(key);

				if (ch != 0)
					AddCharToCurrent(ch);
			}
		}
	}

	bool GUINodeTextArea::OnShortcut(Shortcut sc)
	{
		if (!m_isEditing)
			return false;

		if (sc == Shortcut::CTRL_C || sc == Shortcut::CTRL_X)
		{
			if (m_caretIndexEnd != m_caretIndexStart)
			{
				WString		 finalString = L"";
				const uint32 min		 = Math::Min(m_caretIndexStart, m_caretIndexEnd);
				const uint32 max		 = Math::Max(m_caretIndexStart, m_caretIndexEnd);
				for (uint32 i = min; i < max; i++)
					finalString += m_characters[i].c;

				PlatformProcess::CopyToClipboard(finalString.c_str());

				if (sc == Shortcut::CTRL_X)
					EraseChar();
			}
		}
		else if (sc == Shortcut::CTRL_A)
		{
			SelectAll();
		}
		else if (sc == Shortcut::CTRL_V)
		{
			WString str = L"";
			if (PlatformProcess::TryGetStringFromClipboard(str))
			{
				if (m_caretIndexEnd != m_caretIndexStart)
					EraseChar();

				for (wchar_t ch : str)
					AddCharToCurrent(ch);
			}
		}

		return true;
	}

	void GUINodeTextArea::DrawMultiSelection(int threadID)
	{
		LinaVG::StyleOptions opts;
		opts.color		   = LV4(Theme::TC_CyanAccent);
		opts.color.start.w = opts.color.end.w = 0.2f;

		const uint32 sz = static_cast<uint32>(m_characters.size());

		const uint32 from = m_caretIndexStart;
		const uint32 to	  = m_caretIndexEnd;

		if (from < to)
		{
			const float	  startX = m_characters[from].x;
			const float	  endX	 = to == sz ? (m_characters[sz - 1].x + m_characters[sz - 1].sizeX) : m_characters[to].x;
			const Vector2 start	 = Vector2(m_initialTextPos.x + startX, m_rect.pos.y + m_rect.size.y * 0.5f - m_caretHeight * 0.5f);
			const Vector2 end	 = Vector2(m_initialTextPos.x + endX, start.y + m_caretHeight);
			LinaVG::DrawRect(threadID, LV2(start), LV2(end), opts, 0.0f, m_drawOrder);
		}
		else if (from > to)
		{
			const float	  startX = m_characters[to].x;
			const float	  endX	 = from == sz ? (m_characters[sz - 1].x + m_characters[sz - 1].sizeX) : m_characters[from].x;
			const Vector2 start	 = Vector2(m_initialTextPos.x + startX, m_rect.pos.y + m_rect.size.y * 0.5f - m_caretHeight * 0.5f);
			const Vector2 end	 = Vector2(m_initialTextPos.x + endX, start.y + m_caretHeight);
			LinaVG::DrawRect(threadID, LV2(start), LV2(end), opts, 0.0f, m_drawOrder);
		}
	}

	uint32 GUINodeTextArea::FindCaretIndexFromMouse()
	{
		uint32		  caretIndex	 = 0;
		const Vector2 mp			 = m_window->GetMousePosition();
		const auto&	  lastChar		 = m_characters[m_characters.size() - 1];
		const float	  xAlpha		 = mp.x - m_rect.pos.x;
		const float	  textStartAlpha = m_initialTextPos.x - m_rect.pos.x;
		const float	  textEndAlpha	 = m_initialTextPos.x + lastChar.x + lastChar.sizeX;

		const uint32 sz = static_cast<uint32>(m_characters.size());

		if (xAlpha < textStartAlpha)
			caretIndex = 0;
		else if (xAlpha > textEndAlpha - textStartAlpha)
			caretIndex = sz;
		else
		{
			for (uint32 i = 0; i < sz; i++)
			{
				const auto& cd = m_characters[i];
				if (xAlpha - textStartAlpha > cd.x)
					caretIndex = i;
			}
		}

		return caretIndex;
	}

	void GUINodeTextArea::FinishEditing()
	{
		m_isEditing		  = false;
		m_caretIndexStart = 0;
		m_caretIndexEnd	  = 0;
	}

	void GUINodeTextArea::EraseChar()
	{
		// Debug title
		if (m_caretIndexEnd != m_caretIndexStart)
		{
			const uint32 min = Math::Min(m_caretIndexStart, m_caretIndexEnd);
			const uint32 max = Math::Max(m_caretIndexStart, m_caretIndexEnd);
			m_characters.erase(m_characters.begin() + min, m_characters.begin() + max);
			m_caretIndexStart = m_caretIndexEnd = min;
		}
		else
		{
			if (m_caretIndexEnd != 0)
			{
				m_characters.erase(m_characters.begin() + m_caretIndexEnd - 1);
				m_caretIndexStart = --m_caretIndexEnd;
			}
		}

		RefreshTitle();
	}

	void GUINodeTextArea::AddCharToCurrent(wchar_t ch)
	{
		if (m_caretIndexStart != m_caretIndexEnd)
			EraseChar();

		CharData cd;
		cd.c = ch;
		m_characters.insert(m_characters.begin() + m_caretIndexEnd, cd);
		RefreshTitle();
		m_caretIndexStart = ++m_caretIndexEnd;
	}

	String GUINodeTextArea::Convert(const Vector<wchar_t>& v)
	{
		WString wstr = WString(v.begin(), v.end());
		return Internal::WideStringToString(wstr);
	}

	void GUINodeTextArea::RefreshTitle()
	{
		Vector<wchar_t> data;
		for (auto& ch : m_characters)
			data.push_back(ch.c);
		const String str = Convert(data);
		SetTitle(str);
	}

	void GUINodeTextArea::SelectAll()
	{
		m_isEditing		  = true;
		m_caretIndexStart = 0;
		m_caretIndexEnd	  = static_cast<uint32>(m_characters.size());
	}

} // namespace Lina::Editor
