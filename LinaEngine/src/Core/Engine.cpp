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

#include "Core/Engine.hpp"

#include "Audio/Audio.hpp"
#include "EventSystem/ApplicationEvents.hpp"
#include "EventSystem/MainLoopEvents.hpp"
#include "Log/Log.hpp"
#include "Core/Level.hpp"
#include "Profiling/Profiler.hpp"
#include "Utility/UtilityFunctions.hpp"
#include "Physics/PhysicsMaterial.hpp"
#include "Audio/Audio.hpp"
#include "Math/Math.hpp"
#include "Settings/EngineSettings.hpp"
#include "Settings/RenderSettings.hpp"
#include "Data/HashSet.hpp"
#include "Core/Level.hpp"
#include "Profiling/Profiler.hpp"
#include "Resource/Shader.hpp"
#include "Resource/Model.hpp"
#include "Resource/Material.hpp"
#include "Resource/Texture.hpp"
#include "Core/CommonEngine.hpp"
#include "Reflection/ReflectionSystem.hpp"
#include "Components/RenderableComponent.hpp"
#include "Core/ResourceLoader.hpp"

namespace Lina
{
    Engine* Engine::s_engine = nullptr;

    void Engine::InstallLevel(const String& path, bool async)
    {
        m_renderEngine.Join();
        m_levelManager.InstallLevel(path, async);
    }

    void Engine::Initialize(const InitInfo& initInfo)
    {
        Event::EventSystem::s_eventSystem       = &m_eventSystem;
        Physics::PhysicsEngine::s_physicsEngine = &m_physicsEngine;
        Input::InputEngine::s_inputEngine       = &m_inputEngine;
        Audio::AudioEngine::s_audioEngine       = &m_audioEngine;
        Resources::ResourceManager::s_instance  = &m_resourceManager;
        World::LevelManager::s_instance         = &m_levelManager;
        Graphics::RenderEngine::s_instance      = &m_renderEngine;
        JobSystem::s_instance                   = &m_jobSystem;
        Memory::MemoryManager::s_instance       = &m_memoryManager;

        RegisterResourceTypes();
        m_memoryManager.Initialize();
        m_eventSystem.Initialize();
        m_jobSystem.Initialize();
        m_resourceManager.Initialize();
        m_inputEngine.Initialize();
        m_audioEngine.Initialize();
        m_physicsEngine.Initialize();
        m_levelManager.Initialize();
        m_renderEngine.Initialize(initInfo);

        g_levelTypeID = GetTypeID<World::Level>();

#ifndef LINA_PRODUCTION_BUILD
        if (ApplicationInfo::GetAppMode() == ApplicationMode::Editor)
            m_editor.Initialize();
#endif

        LoadEngineResources();
    }

    void Engine::LoadEngineResources()
    {
#ifndef LINA_PRODUCTION_BUILD
        if (ApplicationInfo::GetAppMode() == ApplicationMode::Editor)
            m_editor.VerifyStaticResources();
#endif

        // Static resources
        const Vector<String> defaultShaders   = m_renderEngine.GetEngineShaderPaths();
        const Vector<String> defaultMaterials = m_renderEngine.GetEngineMaterialPaths();
        const Vector<String> defaultModels    = m_renderEngine.GetEnginePrimitivePaths();
        const Vector<String> defaultTextures  = m_renderEngine.GetEngineTexturePaths();

        for (auto& s : defaultShaders)
            DefaultResources::s_engineResources[GetTypeID<Graphics::Shader>()].push_back(s);

        for (auto& s : defaultMaterials)
            DefaultResources::s_engineResources[GetTypeID<Graphics::Material>()].push_back(s);

        for (auto& s : defaultModels)
            DefaultResources::s_engineResources[GetTypeID<Graphics::Model>()].push_back(s);

        for (auto& s : defaultTextures)
            DefaultResources::s_engineResources[GetTypeID<Graphics::Texture>()].push_back(s);

        DefaultResources::s_engineResources[GetTypeID<EngineSettings>()].push_back("Resources/engine.linasettings");
        DefaultResources::s_engineResources[GetTypeID<RenderSettings>()].push_back("Resources/render.linasettings");
        DefaultResources::s_engineResources[GetTypeID<Audio::Audio>()].push_back("Resources/Engine/Audio/Startup.wav");

        // Debug
        DefaultResources::s_engineResources[GetTypeID<Graphics::Model>()].push_back("Resources/Engine/Models/Tests/Test.fbx");

        m_resourceManager.GetLoader()->LoadEngineResources();

        m_engineSettings = m_resourceManager.GetResource<EngineSettings>("Resources/engine.linasettings");
        m_renderSettings = m_resourceManager.GetResource<RenderSettings>("Resources/render.linasettings");

        // Temp
        //  Graphics::Shader* shader = m_renderEngine.GetEngineShader(Graphics::EngineShaderType::SQFinal);
        // Graphics::Material mat;
        // mat.SetShader(shader);
        // Serialization::SaveToFile<Graphics::Material>("Resources/LitStandard.linamat", mat);
        //
        // shader = m_renderEngine.GetEngineShader(Graphics::EngineShaderType::GUIStandard);
        // mat.SetShader(shader);
        // Serialization::SaveToFile<Graphics::Material>("Resources/GUIStandard.linamat", mat);

        // shader = m_renderEngine.GetEngineShader(Graphics::EngineShaderType::SQFinal);
        // mat.SetShader(shader);
        // Serialization::SaveToFile<Graphics::Material>("Resources/SQFinal.linamat", mat);

        // Graphics::Material mat;
        // Serialization::SaveToFile<Graphics::Material>("Resources/Empty.linamat", mat);

        // mat->SetShader(shader);
        // Serialization::SaveToFile<Graphics::Material>("Resources/SQFinal.linamat", *mat);

        const auto& paths = m_renderEngine.GetEngineMaterialPaths();
        for (auto& p : paths)
        {
            auto mat = m_resourceManager.GetResource<Graphics::Material>(p);
            mat->SaveToFile();
        }

        // auto mat = m_renderEngine.GetEngineMaterial(Graphics::EngineShaderType::GUIText);
        // mat->SetShader(m_renderEngine.GetEngineShader(Graphics::EngineShaderType::GUIText));
        // mat->SaveToFile();
        // int xd = 5;
    }

    void Engine::Run()
    {
        m_deltaTimeArray.fill(-1.0);

        m_running                = true;
        RuntimeInfo::s_startTime = Time::GetCPUTime();
        m_physicsAccumulator     = 0.0f;

        int    frames       = 0;
        int    updates      = 0;
        double totalFPSTime = 0.0f;
        double previousFrameTime;
        double currentFrameTime = 0.0f;

        RuntimeInfo::s_paused          = false;
        RuntimeInfo::s_shouldSkipFrame = false;
        RuntimeInfo::s_isInPlayMode    = ApplicationInfo::GetAppMode() != ApplicationMode::Editor;

        m_levelManager.CreateLevel("Resources/Sandbox/Levels/level2.linalevel");
        m_levelManager.InstallLevel("Resources/Sandbox/Levels/level2.linalevel", false);

        m_levelManager.SaveCurrentLevel();
        // m_engineSettings->m_packagedLevels.push_back("Resources/Sandbox/Levels/level2.linalevel");

        // m_levelManager.GetCurrentLevel()->m_usedResources.push_back(linatl::make_pair(GetTypeID<Graphics::Texture>(), "Resources/Engine/Textures/Tests/empire_diffuse.png"));
        // SetFrameLimit(30);

        m_eventSystem.Trigger<Event::EPreMainLoop>(Event::EPreMainLoop{});
        const String initialTitle = m_renderEngine.m_window->GetTitle();

        while (m_running)
        {
            PROFILER_FRAME_START();

            previousFrameTime = currentFrameTime;
            currentFrameTime  = RuntimeInfo::GetElapsedTime();

            if (m_frameLimit > 0 && !m_firstRun)
            {
                const double diff = currentFrameTime - previousFrameTime;

                if (diff < m_frameLimitSeconds)
                {
                    const double wa = (m_frameLimitSeconds - diff);
                    PROFILER_SCOPE_START("Sleep", PROFILER_THREAD_MAIN);
                    Time::Sleep(wa);
                    PROFILER_SCOPE_END("Sleep", PROFILER_THREAD_MAIN);
                    currentFrameTime += wa;
                }
            }

            RuntimeInfo::s_deltaTime       = (float)(currentFrameTime - previousFrameTime);
            RuntimeInfo::s_smoothDeltaTime = static_cast<float>(SmoothDeltaTime(RuntimeInfo::s_deltaTime));

            // Input
            m_inputEngine.Tick();

            // Window poll
            Graphics::Window::Get()->Tick();

            // Render
            m_renderJob = m_jobSystem.GetMainExecutor().Async([&]() {
                m_renderEngine.Render();
                frames++;
            });

            // Game sim, physics + update etc.
            RunSimulation((float)RuntimeInfo::s_deltaTime);
            updates++;

            // Wait for all.
            m_renderJob.get();

            // Sync previous frame.
            m_renderEngine.SyncData();
            m_eventSystem.Trigger<Event::ESyncThreads>({});

            PROFILER_SCOPE_START("Core Loop Finalize", PROFILER_THREAD_MAIN);

            // Calculate FPS, UPS.
            if (currentFrameTime - totalFPSTime >= 1.0)
            {
                m_frameTime  = RuntimeInfo::s_deltaTime * 1000;
                m_currentFPS = frames;
                m_currentUPS = updates;
                totalFPSTime += 1.0f;
                frames  = 0;
                updates = 0;
            }

            m_renderEngine.m_window->SetTitle(initialTitle + " FPS: " + TO_STRING(m_currentFPS));
            // LINA_TRACE("FPS: {0}", m_currentFPS);

            if (m_firstRun)
                m_firstRun = false;

            PROFILER_SCOPE_END("Core Loop Finalize", PROFILER_THREAD_MAIN);
        }

        m_eventSystem.Trigger<Event::EPostMainLoop>(Event::EPostMainLoop{});

#ifndef LINA_PRODUCTION_BUILD
        if (ApplicationInfo::GetAppMode() == ApplicationMode::Editor)
            m_editor.Shutdown();
#endif
        // m_editor.PackageProject();
        m_memoryManager.PrintStaticBlockInfo();

        LINA_TRACE("[Engine] -> Waiting for all jobs.");
        m_jobSystem.WaitForAll();
        LINA_TRACE("[Engine] -> All jobs finished.");
        LINA_TRACE("[Engine] -> Waiting for render thread.");
        m_renderEngine.Join();
        LINA_TRACE("[Engine] -> Render thread finished.");

        m_levelManager.UninstallCurrent();
        Reflection::Clear();

        // Ending game.
        m_eventSystem.Trigger<Event::EEndGame>(Event::EEndGame{});

        PROFILER_DUMP_FRAME_ANALYSIS("lina_frame_analysis.txt");

        // Shutting down.
        m_resourceManager.Shutdown();
        m_eventSystem.Trigger<Event::EShutdown>(Event::EShutdown{});
        m_renderEngine.Shutdown();
        m_levelManager.Shutdown();
        m_audioEngine.Shutdown();
        m_physicsEngine.Shutdown();
        m_inputEngine.Shutdown();
        m_jobSystem.Shutdown();
        m_eventSystem.Shutdown();
        m_memoryManager.Shutdown();
    }

    void Engine::Stop()
    {
        m_renderJob.cancel();

        if (m_renderJob.valid())
            m_renderJob.get();

        m_renderEngine.Stop();
        m_running = false;
        m_renderEngine.Join();
    }

    void Engine::RunSimulation(float deltaTime)
    {
        // Pause & skip frame controls.
        if (RuntimeInfo::s_paused && !RuntimeInfo::s_shouldSkipFrame)
            return;
        RuntimeInfo::s_shouldSkipFrame = false;

        PROFILER_SCOPE_START("Event: Pre Tick", PROFILER_THREAD_MAIN);
        m_eventSystem.Trigger<Event::EPreTick>(Event::EPreTick{(float)RuntimeInfo::s_deltaTime, RuntimeInfo::s_isInPlayMode});
        PROFILER_SCOPE_END("Event: Pre Tick", PROFILER_THREAD_MAIN);

        // Physics events & physics tick.
        m_physicsAccumulator += deltaTime;
        float physicsStep = m_physicsEngine.GetStepTime();

        if (m_physicsAccumulator >= physicsStep)
        {
            m_physicsAccumulator -= physicsStep;

            PROFILER_SCOPE_START("Event: Pre Physics", PROFILER_THREAD_MAIN);
            m_eventSystem.Trigger<Event::EPrePhysicsTick>(Event::EPrePhysicsTick{});
            PROFILER_SCOPE_END("Event: Pre Physics", PROFILER_THREAD_MAIN);

            PROFILER_SCOPE_START("Physics Simulation", PROFILER_THREAD_MAIN);
            m_physicsEngine.Tick(physicsStep);
            PROFILER_SCOPE_END("Physics Simulation", PROFILER_THREAD_MAIN);

            PROFILER_SCOPE_START("Event: Post Physics", PROFILER_THREAD_MAIN);
            m_eventSystem.Trigger<Event::EPostPhysicsTick>(Event::EPostPhysicsTick{physicsStep, RuntimeInfo::s_isInPlayMode});
            PROFILER_SCOPE_END("Event: Post Physics", PROFILER_THREAD_MAIN);
        }

        // Other main systems (engine or game)
        PROFILER_SCOPE_START("Event: Simulation Tick", PROFILER_THREAD_MAIN);
        m_eventSystem.Trigger<Event::ETick>(Event::ETick{(float)RuntimeInfo::s_deltaTime, RuntimeInfo::s_isInPlayMode});
        PROFILER_SCOPE_END("Event: Simulation Tick", PROFILER_THREAD_MAIN);

        PROFILER_SCOPE_START("Event: Post Simulation Tick", PROFILER_THREAD_MAIN);
        m_eventSystem.Trigger<Event::EPostTick>(Event::EPostTick{(float)RuntimeInfo::s_deltaTime, RuntimeInfo::s_isInPlayMode});
        PROFILER_SCOPE_END("Event: Post Simulation Tick", PROFILER_THREAD_MAIN);

        PROFILER_SCOPE_START("Render Engine Tick", PROFILER_THREAD_MAIN);
        m_renderEngine.Tick();
        PROFILER_SCOPE_END("Render Engine Tick", PROFILER_THREAD_MAIN);
    }

    void Engine::RemoveOutliers(bool biggest)
    {
        double outlier      = biggest ? 0 : 10;
        int    outlierIndex = -1;
        int    indexCounter = 0;
        for (double d : m_deltaTimeArray)
        {
            if (d < 0)
            {
                indexCounter++;
                continue;
            }

            if (biggest)
            {
                if (d > outlier)
                {
                    outlierIndex = indexCounter;
                    outlier      = d;
                }
            }
            else
            {
                if (d < outlier)
                {
                    outlierIndex = indexCounter;
                    outlier      = d;
                }
            }

            indexCounter++;
        }

        if (outlierIndex != -1)
            m_deltaTimeArray[outlierIndex] = m_deltaTimeArray[outlierIndex] * -1.0;
    }

    void Engine::RegisterResourceTypes()
    {
        Vector<String> extensions;

        extensions.push_back("enginesettings");
        m_resourceManager.RegisterResourceType<EngineSettings>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Static,
            .typeName             = "Engine Settings",
            .memChunkCount        = 1,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("rendersettings");
        m_resourceManager.RegisterResourceType<RenderSettings>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Static,
            .typeName             = "Render Settings",
            .memChunkCount        = 1,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("linalevel");
        m_resourceManager.RegisterResourceType<World::Level>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Level,
            .typeName             = "Level",
            .memChunkCount        = 20,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("mp3");
        extensions.push_back("wav");
        extensions.push_back("ogg");
        m_resourceManager.RegisterResourceType<Audio::Audio>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Audio,
            .typeName             = "Audio",
            .memChunkCount        = 100,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("linaphymat");
        m_resourceManager.RegisterResourceType<Physics::PhysicsMaterial>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Physics,
            .typeName             = "Physics Material",
            .memChunkCount        = 30,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("linashader");
        m_resourceManager.RegisterResourceType<Graphics::Shader>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Graphics,
            .typeName             = "Graphics",
            .memChunkCount        = 20,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("linamat");
        m_resourceManager.RegisterResourceType<Graphics::Material>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Graphics,
            .typeName             = "Material",
            .memChunkCount        = 50,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("fbx");
        extensions.push_back("obj");
        m_resourceManager.RegisterResourceType<Graphics::Model>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Models,
            .typeName             = "Model",
            .memChunkCount        = 100,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });

        extensions.clear();
        extensions.push_back("png");
        extensions.push_back("jpg");
        extensions.push_back("jpeg");
        m_resourceManager.RegisterResourceType<Graphics::Texture>(Resources::ResourceTypeData{
            .packageType          = Resources::PackageType::Textures,
            .typeName             = "Texture",
            .memChunkCount        = 200,
            .associatedExtensions = extensions,
            .debugColor           = Color::White,
        });
        // TODO: Font class.
    }

    double Engine::SmoothDeltaTime(double dt)
    {

        if (m_deltaFirstFill < DELTA_TIME_HISTORY)
        {
            m_deltaFirstFill++;
        }
        else if (!m_deltaFilled)
            m_deltaFilled = true;

        m_deltaTimeArray[m_deltaTimeArrOffset] = dt;
        m_deltaTimeArrOffset++;

        if (m_deltaTimeArrOffset == DELTA_TIME_HISTORY)
            m_deltaTimeArrOffset = 0;

        if (!m_deltaFilled)
            return dt;

        // Remove the biggest & smalles 2 deltas.
        RemoveOutliers(true);
        RemoveOutliers(true);
        RemoveOutliers(false);
        RemoveOutliers(false);

        double avg   = 0.0;
        int    index = 0;
        for (double d : m_deltaTimeArray)
        {
            if (d < 0.0)
            {
                m_deltaTimeArray[index] = m_deltaTimeArray[index] * -1.0;
                index++;
                continue;
            }

            avg += d;
            index++;
        }

        avg /= static_cast<double>(DELTA_TIME_HISTORY) - 4.0;

        return avg;
    }

} // namespace Lina
