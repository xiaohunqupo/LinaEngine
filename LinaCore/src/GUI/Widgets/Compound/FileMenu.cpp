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

#include "Core/GUI/Widgets/Compound/FileMenu.hpp"
#include "Core/GUI/Widgets/Compound/Popup.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/Icon.hpp"
#include "Core/GUI/Widgets/Primitives/ShapeRect.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/GUI/Widgets/Primitives/Selectable.hpp"
#include "Common/Math/Math.hpp"

namespace Lina
{
	void FileMenuItem::Initialize()
	{
		if (m_itemData.isDivider)
		{
			ShapeRect* rect = m_manager->Allocate<ShapeRect>("Shape");
			rect->GetFlags().Set(WF_SIZE_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
			rect->SetAlignedSize(Vector2(1.0f, 0.25f));
			rect->SetAlignedPosY(0.5f);
			rect->SetPosAlignmentSourceY(PosAlignmentSource::Center);
			rect->GetProps().colorStart = rect->GetProps().colorEnd = Theme::GetDef().outlineColorBase;
			AddChild(rect);
			return;
		}

		Text* txt = m_manager->Allocate<Text>("Text");
		txt->GetFlags().Set(WF_POS_ALIGN_Y);
		txt->SetAlignedPosY(0.5f);
		txt->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		txt->GetProps().text		  = m_itemData.text;
		txt->GetProps().colorDisabled = Theme::GetDef().silent2;
		AddChild(txt);
		m_text = txt;

		if (m_itemData.hasDropdown)
		{
			Icon* dd = m_manager->Allocate<Icon>("DD");
			dd->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
			dd->SetAlignedPos(Vector2(1.0f, 0.5f));
			dd->SetPosAlignmentSourceX(PosAlignmentSource::End);
			dd->SetPosAlignmentSourceY(PosAlignmentSource::Center);
			dd->GetProps().icon = m_itemData.dropdownIcon;
			AddChild(dd);
		}

		if (!m_itemData.altText.empty())
		{
			Text* altTxt = m_manager->Allocate<Text>("Text");
			altTxt->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
			altTxt->SetAlignedPos(Vector2(1.0f, 0.5f));
			altTxt->SetPosAlignmentSourceX(PosAlignmentSource::End);
			altTxt->SetPosAlignmentSourceY(PosAlignmentSource::Center);
			altTxt->GetProps().text	 = m_itemData.altText;
			altTxt->GetProps().font	 = Theme::GetDef().altFont;
			altTxt->GetProps().color = Theme::GetDef().silent2;
			AddChild(altTxt);
			m_altText = altTxt;
		}

		Widget::Initialize();
	}

	void FileMenuItem::PreTick()
	{
		DirectionalLayout::PreTick();

		if (m_itemData.hasDropdown && m_isHovered && m_subPopup == nullptr)
		{
			Vector<Data> data;
			m_ownerMenu->GetListener()->OnGetItemData(TO_SID(m_itemData.text), data);
			m_subPopup = m_ownerMenu->CreatePopup(Vector2(m_rect.GetEnd().x, GetPosY()), data);

			m_subPopup->GetProps().onDestructed = [this]() { m_subPopup = nullptr; };
		}

		if (m_subPopup != nullptr && !m_isHovered)
		{
			for (auto* sibling : m_parent->GetChildren())
			{
				if (sibling == this)
					continue;

				if (sibling->GetIsHovered())
				{
					m_manager->RemoveFromForeground(m_subPopup);
					m_manager->Deallocate(m_subPopup);
					m_subPopup = nullptr;
					break;
				}
			}
		}
	}

	void FileMenu::Construct()
	{
		GetProps().direction = DirectionOrientation::Horizontal;
		GetFlags().Set(WF_SIZE_X_TOTAL_CHILDREN);
		DirectionalLayout::Construct();
	}

	void FileMenu::PreTick()
	{
		DirectionalLayout::PreTick();

		int32 idx = 0;
		for (auto* b : m_buttons)
		{
			b->GetProps().colorDefaultStart = b->GetProps().colorDefaultEnd = b == m_subPopupOwner ? Theme::GetDef().accentPrimary2 : Color(0, 0, 0, 0);

			if (m_subPopup != nullptr && b != m_subPopupOwner && b->GetRect().IsPointInside(m_lgxWindow->GetMousePosition()))
			{
				m_manager->RemoveFromForeground(m_subPopup);
				m_manager->Deallocate(m_subPopup);
				m_subPopup		= nullptr;
				m_subPopupOwner = nullptr;

				Vector<FileMenuItem::Data> itemData;
				m_listener->OnGetItemData(TO_SID(b->GetText()->GetProps().text), itemData);
				m_subPopup							= CreatePopup(Vector2(b->GetPosX(), b->GetRect().GetEnd().y), itemData);
				m_subPopup->GetProps().onDestructed = [this]() {
					m_subPopup		= nullptr;
					m_subPopupOwner = nullptr;
				};
				m_subPopupOwner = b;
			}

			idx++;
		}
	}

	void FileMenu::Initialize()
	{
		int32 idx = 0;
		for (const auto& str : m_fileMenuProps.buttons)
		{
			Button* btn = m_manager->Allocate<Button>("FMButton");
			btn->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y | WF_SIZE_X_MAX_CHILDREN);
			btn->SetAlignedPosY(0.0f);
			btn->SetAlignedSizeY(1.0f);
			btn->GetChildMargins() = {.left = Theme::GetDef().baseIndent, .right = Theme::GetDef().baseIndent};

			btn->GetProps().outlineThickness  = 0.0f;
			btn->GetProps().rounding		  = 0.0f;
			btn->GetProps().colorDefaultStart = Color(0.0f, 0.0f, 0.0f, 0.0f);
			btn->GetProps().colorDefaultEnd	  = Color(0.0f, 0.0f, 0.0f, 0.0f);
			btn->GetProps().colorPressed	  = Theme::GetDef().accentPrimary0;
			btn->GetProps().colorHovered	  = Theme::GetDef().accentPrimary1;

			btn->GetProps().onClicked = [this, btn, str]() {
				Vector<FileMenuItem::Data> itemData;
				m_listener->OnGetItemData(TO_SID(str), itemData);
				m_subPopup							= CreatePopup(Vector2(btn->GetPosX(), btn->GetRect().GetEnd().y), itemData);
				m_subPopup->GetProps().onDestructed = [this]() {
					m_subPopup		= nullptr;
					m_subPopupOwner = nullptr;
				};
				m_subPopupOwner = btn;
			};

			btn->GetText()->GetProps().text = str;
			m_buttons.push_back(btn);
			AddChild(btn);
			idx++;
		}

		DirectionalLayout::Initialize();
	}

	DirectionalLayout* FileMenu::CreatePopup(const Vector2& pos, const Vector<FileMenuItem::Data>& subItemData)
	{
		DirectionalLayout* popup = WidgetUtility::BuildLayoutForPopups(this);
		popup->SetPos(pos);

		m_manager->AddToForeground(popup);
		m_manager->SetForegroundDim(0.0f);

		for (const auto& subItem : subItemData)
		{
			FileMenuItem* it  = m_manager->Allocate<FileMenuItem>("FMItem");
			it->m_ownerMenu	  = this;
			it->GetItemData() = subItem;

			if (!subItem.isDivider)
			{
				it->GetProps().useHoverColor   = true;
				it->GetProps().receiveInput	   = true;
				it->GetProps().backgroundStyle = DirectionalLayout::BackgroundStyle::Default;
				it->GetProps().colorHovered	   = Theme::GetDef().accentPrimary1;
				it->SetFixedSizeY(Theme::GetDef().baseItemHeight);
				it->GetChildMargins() = {.left = Theme::GetDef().baseIndent, .right = Theme::GetDef().baseIndent};
			}
			else
				it->SetFixedSizeY(Theme::GetDef().baseItemHeight * 0.5f);

			it->GetProps().colorBackgroundStart = it->GetProps().colorBackgroundEnd = Color(0, 0, 0, 0);
			it->SetChildPadding(Theme::GetDef().baseIndent);
			it->GetFlags().Set(WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y | WF_POS_ALIGN_X);
			it->SetAlignedSizeX(1.0f);
			it->SetAlignedPosX(0.0f);

			const StringID sid = TO_SID(subItem.text);

			it->GetProps().onClicked = [sid, popup, this]() {
				if (m_listener->OnItemClicked(sid))
				{
					m_manager->RemoveFromForeground(popup);
					m_manager->Deallocate(popup);
				}
			};

			if (m_listener->IsItemDisabled(sid))
				it->SetIsDisabled(true);

			popup->AddChild(it);
		}

		popup->Initialize();

		float maxTextSize = 0.0f;

		for (const auto& c : popup->GetChildren())
		{
			auto* fmi	  = static_cast<FileMenuItem*>(c);
			auto* text	  = fmi->GetText();
			auto* altText = fmi->GetAltText();

			float size = 0.0f;

			if (text)
				size += text->GetSizeX();

			if (altText)
				size += altText->GetSizeX() + fmi->GetChildPadding();

			maxTextSize = Math::Max(maxTextSize, size);
		}

		popup->SetFixedSizeX(Math::Max(Theme::GetDef().baseItemHeight * 8, (maxTextSize + popup->GetChildMargins().left + popup->GetChildMargins().right) * 1.25f));

		return popup;
	}

} // namespace Lina
