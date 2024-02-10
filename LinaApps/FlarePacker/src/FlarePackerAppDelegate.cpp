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

#include "Core/Lina.hpp"
#include "FlarePackerAppDelegate.hpp"
#include "Common/GUI/Widgets/Layout/FreeRoam.hpp"
#include "Common/GUI/Widgets/Layout/Column.hpp"
#include "Common/GUI/Widgets/Layout/Row.hpp"
#include "Common/GUI/WidgetAllocator.hpp"
#include "Common/GUI/Widgets/Container/Box.hpp"

namespace Lina
{

	SystemInitializationInfo Lina_GetInitInfo()
	{
		return SystemInitializationInfo{
			.appName			 = "Flare Packer",
			.windowWidth		 = 1440,
			.windowHeight		 = 1440,
			.windowStyle		 = LinaGX::WindowStyle::WindowedApplication,
			.appListener		 = new Lina::FlarePackerAppDelegate(),
			.resourceManagerMode = Lina::ResourceManagerMode::File,
		};
	}

	void FlarePackerAppDelegate::OnSystemEvent(SystemEvent eventType, const Event& ev)
	{
	}

	void FlarePackerAppDelegate::RenderSurfaceOverlay(LinaGX::CommandStream* cmdStream, LinaGX::Window* window, int32 threadIndex)
	{
		FreeRoam* root = WidgetAllocator::Get().Allocate<FreeRoam>(threadIndex);
		root->SetSize(window->GetSize());

		Row* col = root->AllocateChild<Row>();
		col->SetPadding(125);
		col->SetMargins(MARGINS_EQ(10));
		col->SetMainAlignment(MainAlignment::Free);
		col->SetCrossAlignment(CrossAlignment::Start);

		Box* box1 = col->AllocateChild<Box>();
		box1->SetSize(Vector2i(40, 40));

		Box* box2 = col->AllocateChild<Box>();
		box2->SetSize(Vector2i(40, 20));

		Box* box3 = col->AllocateChild<Box>();
		box3->SetSize(Vector2i(40, 10));

		Box* box4 = col->AllocateChild<Box>();
		box4->SetSize(Vector2i(40, 40));

		root->CalculateDesiredSize();
		root->Draw();
	}

} // namespace Lina
