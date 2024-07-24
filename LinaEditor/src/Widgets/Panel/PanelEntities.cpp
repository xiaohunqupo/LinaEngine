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

#include "Editor/Widgets/Panel/PanelEntities.hpp"
#include "Editor/Widgets/Testbed.hpp"
#include "Editor/EditorLocale.hpp"
#include "Editor/Widgets/Testbed.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Common/System/System.hpp"
#include "Core/World/WorldManager.hpp"
#include "Core/GUI/Widgets/Layout/DirectionalLayout.hpp"
#include "Core/GUI/Widgets/Layout/ScrollArea.hpp"
#include "Core/GUI/Widgets/Layout/FoldLayout.hpp"
#include "Core/GUI/Widgets/Primitives/InputField.hpp"
#include "Core/GUI/Widgets/Primitives/Selectable.hpp"
#include "Core/GUI/Widgets/Primitives/Icon.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"

namespace Lina::Editor
{
	void PanelEntities::Construct()
	{
		m_editor	   = Editor::Get();
		m_worldManager = m_editor->GetSystem()->CastSubsystem<WorldManager>(SubsystemType::WorldManager);

		DirectionalLayout* layout	 = m_manager->Allocate<DirectionalLayout>("Layout");
		layout->GetProps().direction = DirectionOrientation::Vertical;
		layout->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		layout->SetAlignedPos(Vector2::Zero);
		layout->SetAlignedSize(Vector2::One);
		layout->GetChildMargins() = TBLR::Eq(Theme::GetDef().baseIndent);
		layout->SetChildPadding(Theme::GetDef().baseIndent);
		AddChild(layout);

		InputField* search = m_manager->Allocate<InputField>("Search");
		search->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y);
		search->SetAlignedPosX(0.0f);
		search->SetAlignedSizeX(1.0f);
		search->SetFixedSizeY(Theme::GetDef().baseItemHeight);
		search->GetProps().usePlaceHolder  = true;
		search->GetProps().placeHolderText = Locale::GetStr(LocaleStr::Search);
		search->GetProps().rounding		   = 0.0f;
		layout->AddChild(search);

		Testbed* tb = m_manager->Allocate<Testbed>("TB");
		tb->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		tb->SetAlignedPosX(0.0f);
		tb->SetAlignedSize(Vector2(1.0f, 0.0f));
		layout->AddChild(tb);
	}

	void PanelEntities::Destruct()
	{
	}

	void PanelEntities::PreTick()
	{
		auto* world = m_worldManager->GetMainWorld();

		if (world != m_world)
		{
			m_world = world;
			m_world->AddListener(this);
		}
	}

	void PanelEntities::Tick(float dt)
	{
	}

	void PanelEntities::Draw()
	{
		LinaVG::StyleOptions opts;
		opts.color = Theme::GetDef().background1.AsLVG4();
		m_lvg->DrawRect(m_rect.pos.AsLVG(), m_rect.GetEnd().AsLVG(), opts, 0.0f, m_drawOrder);
		Widget::Draw();
	}

	bool PanelEntities::OnFileMenuItemClicked(FileMenu* filemenu, StringID sid, void* userData)
	{
		return true;
	}

	void PanelEntities::OnFileMenuGetItems(FileMenu* filemenu, StringID sid, Vector<FileMenuItem::Data>& outData, void* userData)
	{
	}
} // namespace Lina::Editor
