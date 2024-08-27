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

#include "Editor/Widgets/Panel/PanelFactory.hpp"
#include "Editor/Widgets/Panel/PanelEntities.hpp"
#include "Editor/Widgets/Panel/PanelWorld.hpp"
#include "Editor/Widgets/Panel/PanelResources.hpp"
#include "Editor/Widgets/Panel/PanelPerformance.hpp"
#include "Editor/Widgets/Panel/PanelWidgetEditor.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"

namespace Lina::Editor
{

	Panel* PanelFactory::CreatePanel(Widget* source, PanelType type, StringID subData)
	{
		Panel* panel = nullptr;

		switch (type)
		{
		case PanelType::Performance:
			panel = source->GetWidgetManager()->Allocate<PanelPerformance>("Performance");
			break;
		case PanelType::Resources:
			panel = source->GetWidgetManager()->Allocate<PanelResources>("Resources");
			break;
		case PanelType::World:
			panel = source->GetWidgetManager()->Allocate<PanelWorld>("World");
			break;
		case PanelType::Entities:
			panel = source->GetWidgetManager()->Allocate<PanelEntities>("Entities");
			break;
		case PanelType::WidgetEditor:
			panel = source->GetWidgetManager()->Allocate<PanelWidgetEditor>("Widget Editor");
			break;
		default:
			break;
		}

		panel->GetWidgetProps().drawBackground		 = true;
		panel->GetWidgetProps().outlineThickness	 = 0.0f;
		panel->GetWidgetProps().rounding			 = 0.0f;
		panel->GetWidgetProps().colorBackground		 = Theme::GetDef().background1;
		panel->GetWidgetProps().dropshadow.enabled	 = true;
		panel->GetWidgetProps().dropshadow.direction = Direction::Top;
		panel->GetWidgetProps().dropshadow.isInner	 = true;
		panel->GetWidgetProps().dropshadow.color.w	 = 0.25f;
		panel->GetWidgetProps().dropshadow.steps	 = 8;

		panel->GetFlags().Set(WF_CONTROL_MANAGER);
		panel->Initialize();
		return panel;
	}
} // namespace Lina::Editor
