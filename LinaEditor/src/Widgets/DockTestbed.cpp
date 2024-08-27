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

#include "Editor/Widgets/DockTestbed.hpp"
#include "Editor/Widgets/Docking/DockArea.hpp"
#include "Editor/Widgets/Testbed.hpp"
#include "Editor/Widgets/Panel/PanelFactory.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"

namespace Lina::Editor
{

	void DockTestbed::Construct()
	{
		GetWidgetProps().debugName = "DockTestBed";
		GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		SetAlignedPos(Vector2::Zero);
		SetAlignedSize(Vector2::One);

		// Testbed* tb = Allocate<Testbed>("Testbed");
		// Widget* tb = m_manager->Allocate<Widget>("Testbed");
		// area->AddChild(tb);

		DockArea* area = m_manager->Allocate<DockArea>("DockArea");
		area->SetAlignedPos(Vector2::Zero);
		area->SetAlignedSize(Vector2::One);
		area->AddPanel(PanelFactory::CreatePanel(area, PanelType::Resources, 0));
		AddChild(area);
	}

	void DockTestbed::Tick(float delta)
	{
	}
} // namespace Lina::Editor
