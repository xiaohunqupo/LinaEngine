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

#include "Core/GUI/Widgets/Layout/SelectableListLayout.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/GUI/Widgets/Layout/ScrollArea.hpp"
#include "Core/GUI/Widgets/Layout/DirectionalLayout.hpp"
#include "Core/GUI/Widgets/Layout/GridLayout.hpp"
#include "Core/GUI/Widgets/Layout/FoldLayout.hpp"
#include "Core/GUI/Widgets/Primitives/Selectable.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/Icon.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Common/Math/Math.hpp"
#include "Common/Platform/LinaVGIncl.hpp"

namespace Lina
{
	void SelectableListLayout::Construct()
	{
	}

	void SelectableListLayout::Initialize()
	{
		ScrollArea* scroll = m_manager->Allocate<ScrollArea>("Scroll");
		scroll->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		scroll->SetAlignedPos(Vector2::Zero);
		scroll->SetAlignedSize(Vector2::One);
		scroll->GetProps().direction = DirectionOrientation::Vertical;
		AddChild(scroll);

		if (!m_props.useGridAsLayout)
		{
			DirectionalLayout* dirLayout	= m_manager->Allocate<DirectionalLayout>("Layout");
			dirLayout->GetProps().direction = DirectionOrientation::Vertical;
			dirLayout->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			dirLayout->SetAlignedPos(Vector2::Zero);
			dirLayout->SetAlignedSize(Vector2::One);
			dirLayout->GetProps().backgroundStyle	   = DirectionalLayout::BackgroundStyle::Default;
			dirLayout->GetProps().colorBackgroundStart = dirLayout->GetProps().colorBackgroundEnd = Theme::GetDef().background0;
			dirLayout->GetProps().colorOutline													  = Theme::GetDef().accentPrimary0;
			scroll->AddChild(dirLayout);
			m_layout = dirLayout;
		}
		else
		{
			GridLayout* gridLayout = m_manager->Allocate<GridLayout>("Layout");
			gridLayout->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			gridLayout->SetAlignedPos(Vector2::Zero);
			gridLayout->SetAlignedSize(Vector2::One);
			gridLayout->GetProps().colorBackground = Theme::GetDef().background0;
			gridLayout->GetProps().colorOutline	   = Theme::GetDef().accentPrimary0;
			scroll->AddChild(gridLayout);
			m_layout = gridLayout;
		}

		RefreshItems();
	}

	void SelectableListLayout::RefreshItems()
	{
		if (!m_listener)
			return;

		Vector<SelectableListItem> items;
		m_listener->SelectableListFillItems(items);

		for (const auto& it : items)
			m_layout->AddChild(CreateItem(it, 0));
	}

	Widget* SelectableListLayout::CreateItem(const SelectableListItem& item, uint8 level)
	{
		FoldLayout* fold = m_manager->Allocate<FoldLayout>("Fold");
		fold->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X);
		fold->SetAlignedPosX(0.0f);
		fold->SetAlignedSizeX(1.0f);
		fold->SetUserData(item.userData);

		Selectable* selectable = m_manager->Allocate<Selectable>("Selectable");
		selectable->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y);
		selectable->SetAlignedPosX(0.0f);
		selectable->SetAlignedSizeX(1.0f);
		selectable->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		selectable->SetLocalControlsManager(m_layout);
		selectable->GetProps().colorOutline = Theme::GetDef().accentPrimary0;
		fold->AddChild(selectable);

		selectable->GetProps().onRightClick = [this, item](Selectable* s) { m_listener->SelectableListContextMenu(item.userData); };

		selectable->GetProps().onInteracted = [fold](Selectable*) { fold->SetIsUnfolded(!fold->GetIsUnfolded()); };

		selectable->SetOnGrabbedControls([this]() {});

		selectable->GetProps().onDockedOut = [this, selectable, item, level]() {
			DirectionalLayout* payload		= m_manager->Allocate<DirectionalLayout>();
			payload->GetChildMargins().left = Theme::GetDef().baseIndent;
			payload->SetUserData(item.userData);

			Text* text = m_manager->Allocate<Text>();
			text->GetFlags().Set(WF_POS_ALIGN_Y);
			text->SetAlignedPosY(0.5f);
			text->SetPosAlignmentSourceY(PosAlignmentSource::Center);
			text->GetProps().text = item.title;
			payload->AddChild(text);
			payload->Initialize();

			m_listener->SelectableListOnPayload(payload);
		};

		DirectionalLayout* layout = m_manager->Allocate<DirectionalLayout>("Layout");
		layout->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y);
		layout->SetAlignedPos(Vector2::Zero);
		layout->SetAlignedSizeX(1.0f);
		layout->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		layout->SetChildPadding(Theme::GetDef().baseIndentInner);
		layout->GetChildMargins().left = Theme::GetDef().baseIndent + Theme::GetDef().baseIndent * level;
		selectable->AddChild(layout);

		Icon* dropdown				   = m_manager->Allocate<Icon>("Dropdown");
		dropdown->GetProps().icon	   = !fold->GetIsUnfolded() ? m_props.dropdownIconFolded : m_props.dropdownIconUnfolded;
		dropdown->GetProps().textScale = 0.3f;
		dropdown->GetFlags().Set(WF_POS_ALIGN_Y);
		dropdown->SetAlignedPosY(0.5f);
		dropdown->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		dropdown->GetProps().onClicked = [fold]() { fold->SetIsUnfolded(!fold->GetIsUnfolded()); };
		dropdown->SetVisible(item.hasChildren);
		layout->AddChild(dropdown);

		Text* title = WidgetUtility::BuildEditableText(this, true, []() {});
		title->GetFlags().Set(WF_POS_ALIGN_Y);
		title->SetAlignedPosY(0.5f);
		title->SetPosAlignmentSourceY(PosAlignmentSource::Center);
		title->GetProps().text = item.title;
		layout->AddChild(title);

		fold->GetProps().onFoldChanged = [dropdown, fold, item, level, this](bool unfolded) {
			dropdown->GetProps().icon = !unfolded ? m_props.dropdownIconFolded : m_props.dropdownIconUnfolded;
			dropdown->CalculateIconSize();

			if (!unfolded)
			{
				auto* first = fold->GetChildren().front();

				for (auto* c : fold->GetChildren())
				{
					if (c == first)
						continue;
					m_manager->Deallocate(c);
				}

				fold->RemoveAllChildren();
				fold->AddChild(first);
			}
			else
			{
				Vector<SelectableListItem> subItems;
				m_listener->SelectableListFillSubItems(subItems, item.userData);

				for (const auto& subItem : subItems)
				{
					auto* child = CreateItem(subItem, level + 1);
					fold->AddChild(child);
					child->Initialize();
				}
			}
		};

		fold->SetIsUnfolded(item.startUnfolded);
		fold->Initialize();
		return fold;
	}

} // namespace Lina
