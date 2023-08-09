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

#include "Core/Application.hpp"
#include "Core/SystemInfo.hpp"
#include "Profiling/Profiler.hpp"
#include "Core/PlatformProcess.hpp"
#include "Core/PlatformTime.hpp"
#include "Math/Math.hpp"
#include "System/IPlugin.hpp"
#include "Core/CoreResourcesRegistry.hpp"
#include "Graphics/Core/CommonGraphics.hpp"
#include "Graphics/Core/GfxManager.hpp"
#include "Platform/LinaGXIncl.hpp"

#include "Graphics/Interfaces/IGUIDrawer.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"
#include "Graphics/Core/SurfaceRenderer.hpp"

namespace Lina
{

	class TestGUIDrawer : public IGUIDrawer
	{
	public:
		TestGUIDrawer() {};
		virtual ~TestGUIDrawer() = default;

		virtual void DrawGUI(int threadID)
		{
			LinaVG::StyleOptions opts;
			LinaVG::DrawRect(LinaVG::Vec2(100, 100), LinaVG::Vec2(500, 500), opts, 0.0f, 1);
		}
	};

	void Application::Initialize(const SystemInitializationInfo& initInfo)
	{
		auto& resourceManager = m_engine.GetResourceManager();

		// Platform setup
		{
			PlatformTime::QueryFreq();
			SystemInfo::SetAppStartCycles(PlatformTime::GetCPUCycles());
			PROFILER_REGISTER_THREAD("Main");
			SetupEnvironment();
		}

		// pre-init priority resources, rendering systems & window
		{
			m_engine.PreInitialize(initInfo);
			resourceManager.LoadResources(resourceManager.GetPriorityResources());
			resourceManager.WaitForAll();
			CreateMainWindow(initInfo);
		}

		m_engine.Initialize(initInfo);
		LoadPlugins();
		OnInited();
	}

	void Application::SetupEnvironment()
	{
		auto& resourceManager = m_engine.GetResourceManager();
		// SetApplicationMode(ApplicationMode::Standalone);
		// resourceManager.SetMode(ResourceManagerMode::Package);
		SetApplicationMode(ApplicationMode::Standalone);
		resourceManager.SetMode(ResourceManagerMode::File);
		m_coreResourceRegistry = new CoreResourcesRegistry();
		m_coreResourceRegistry->RegisterResourceTypes(resourceManager);
		resourceManager.SetPriorityResources(m_coreResourceRegistry->GetPriorityResources());
		resourceManager.SetPriorityResourcesMetadata(m_coreResourceRegistry->GetPriorityResourcesMetadata());
		resourceManager.SetCoreResources(m_coreResourceRegistry->GetCoreResources());
		resourceManager.SetCoreResourcesMetadata(m_coreResourceRegistry->GetCoreResourcesMetadata());
		// SetFrameCap(16667);
		SetFixedTimestep(10000);
		SetFixedTimestep(true);
	}

	void Application::CreateMainWindow(const SystemInitializationInfo& initInfo)
	{
		auto window = m_engine.GetLGXWrapper().CreateApplicationWindow(LINA_MAIN_SWAPCHAIN, initInfo.appName, Vector2i::Zero, Vector2ui(initInfo.windowWidth, initInfo.windowHeight), initInfo.windowStyle == LinaGX::WindowStyle::Borderless);
		window->CenterPositionToCurrentMonitor();
		window->SetCallbackClose([&]() { m_exitRequested = true; });

		auto		   sf  = m_engine.GetGfxManager()->GetSurfaceRenderer(LINA_MAIN_SWAPCHAIN);
		IGUIDrawer* heh = new TestGUIDrawer();
		sf->SetGUIDrawer(heh);
	}

	void Application::OnInited()
	{
		auto& resourceManager = m_engine.GetResourceManager();
		resourceManager.LoadResources(resourceManager.GetCoreResources());
		resourceManager.WaitForAll();
		auto window = m_engine.GetLGXWrapper().GetWindowManager()->GetWindow(LINA_MAIN_SWAPCHAIN);
		window->SetVisible(true);
	}

	void Application::LoadPlugins()
	{
		PlatformProcess::LoadPlugin("GamePlugin.dll", m_engine.GetInterface(), &m_engine);
	}

	void Application::UnloadPlugins()
	{
		PlatformProcess::UnloadPlugin("GamePlugin.dll", &m_engine);
	}

	void Application::PreTick()
	{
		PROFILER_FRAME_START();
		PROFILER_FUNCTION();
		m_engine.PreTick();
	}

	void Application::Poll()
	{
		PROFILER_FUNCTION();
		m_engine.Poll();
	}

	void Application::Tick()
	{
		PROFILER_FUNCTION();

		m_engine.Tick();

		SystemInfo::SetFrames(SystemInfo::GetFrames() + 1);
		SystemInfo::SetAppTime(SystemInfo::GetAppTime() + SystemInfo::GetDeltaTime());

		// Yield-CPU check.
		if (!SystemInfo::GetAppHasFocus())
			PlatformTime::Sleep(0);
	}

	void Application::Shutdown()
	{
		m_engine.GetLGXWrapper().DestroyApplicationWindow(LINA_MAIN_SWAPCHAIN);
		UnloadPlugins();
		m_engine.Shutdown();
		delete m_coreResourceRegistry;
	}

	void Application::SetApplicationMode(ApplicationMode mode)
	{
		SystemInfo::SetApplicationMode(mode);
	}
	void Application::SetFrameCap(int64 microseconds)
	{
		SystemInfo::SetFrameCap(microseconds);
	}
	void Application::SetFixedTimestep(int64 microseconds)
	{
		SystemInfo::SetFixedTimestep(microseconds);
	}

} // namespace Lina
