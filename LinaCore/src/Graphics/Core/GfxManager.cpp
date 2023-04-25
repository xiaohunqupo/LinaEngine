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

#include "Graphics/Core/GfxManager.hpp"
#include "Graphics/Core/SurfaceRenderer.hpp"
#include "Graphics/Core/WorldRenderer.hpp"
#include "Graphics/Interfaces/ISwapchain.hpp"
#include "Graphics/Interfaces/IGfxResourceCPU.hpp"
#include "Graphics/Resource/Material.hpp"
#include "Graphics/Resource/Model.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Graphics/Resource/Font.hpp"
#include "System/ISystem.hpp"
#include "Profiling/Profiler.hpp"
#include "Resources/Core/ResourceManager.hpp"
#include "FileSystem/FileSystem.hpp"
#include "Graphics/Data/RenderData.hpp"
#include "Graphics/Platform/RendererIncl.hpp"
#include "Core/SystemInfo.hpp"
#include "Graphics/Resource/TextureSampler.hpp"
#include "Graphics/Core/GUIBackend.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"

namespace Lina
{
	GfxManager::GfxManager(const SystemInitializationInfo& initInfo, ISystem* sys) : ISubsystem(sys, SubsystemType::GfxManager), m_meshManager(this)
	{
		m_resourceManager = sys->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		m_renderer		  = new Renderer(initInfo, this);
		m_resourceManager->AddListener(this);
		m_meshManager.Initialize(m_renderer->GetUploadContext());

		for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data			  = m_dataPerFrame[i];
			data.globalDataBuffer = m_renderer->CreateCPUResource(sizeof(GPUGlobalData), CPUResourceHint::ConstantBuffer, L"Global Data Buffer");
			data.globalDataBuffer->BufferData(&m_globalData, sizeof(GPUGlobalData));
		}

		// GUIBackend
		{
			m_guiBackend						  = new GUIBackend(this);
			LinaVG::Config.globalFramebufferScale = 1.0f;
			LinaVG::Config.globalAAMultiplier	  = 1.0f;
			LinaVG::Config.gcCollectInterval	  = 4000;
			LinaVG::Config.textCachingEnabled	  = true;
			LinaVG::Config.textCachingSDFEnabled  = true;
			LinaVG::Config.textCacheReserve		  = 10000;
			LinaVG::Config.useUnicodeEncoding	  = false;
			LinaVG::Config.errorCallback		  = [](const std::string& err) { LINA_ERR(err.c_str()); };
			LinaVG::Config.logCallback			  = [](const std::string& log) { LINA_TRACE(log.c_str()); };
			LinaVG::Backend::BaseBackend::SetBackend(m_guiBackend);
			LinaVG::Initialize();
		}
	}

	void GfxManager::Initialize(const SystemInitializationInfo& initInfo)
	{
		// Samplers
		{
			TextureSampler* defaultSampler = new TextureSampler(m_resourceManager, true, "Resource/Core/Samplers/DefaultSampler.linasampler", DEFAULT_SAMPLER_SID);
			SamplerData		samplerData;
			samplerData.minFilter	= Filter::Anisotropic;
			samplerData.magFilter	= Filter::Anisotropic;
			samplerData.mode		= SamplerAddressMode::Repeat;
			samplerData.anisotropy	= 6;
			samplerData.borderColor = Color::White;
			samplerData.mipLodBias	= 0.0f;
			samplerData.minLod		= 0.0f;
			samplerData.maxLod		= 30.0f; // upper limit
			defaultSampler->SetSamplerData(samplerData);

			TextureSampler* defaultGUISampler = new TextureSampler(m_resourceManager, true, "Resource/Core/Samplers/DefaultGUISampler.linasampler", DEFAULT_GUI_SAMPLER_SID);
			samplerData.minFilter			  = Filter::Anisotropic;
			samplerData.magFilter			  = Filter::Anisotropic;
			samplerData.mipLodBias			  = -1.0f;
			defaultGUISampler->SetSamplerData(samplerData);

			TextureSampler* defaultGUITextSampler = new TextureSampler(m_resourceManager, true, "Resource/Core/Samplers/DefaultGUITextSampler.linasampler", DEFAULT_GUI_TEXT_SAMPLER_SID);
			samplerData.minFilter				  = Filter::Nearest;
			samplerData.magFilter				  = Filter::Anisotropic;
			samplerData.mipLodBias				  = 0.0f;
			defaultGUITextSampler->SetSamplerData(samplerData);

			m_engineSamplers.push_back(defaultSampler);
			m_engineSamplers.push_back(defaultGUISampler);
			m_engineSamplers.push_back(defaultGUITextSampler);
		}

		// Shaders & materials
		{
			constexpr uint32 engineShaderCount			= 1;
			const String	 shaders[engineShaderCount] = {"Resources/Core/Shaders/LitStandard.linashader"};

			for (uint32 i = 0; i < engineShaderCount; i++)
			{
				const StringID shaderSID	= TO_SID(shaders[i]);
				const String   materialPath = "Resources/Core/Materials/" + FileSystem::GetFilenameOnlyFromPath(shaders[i]) + ".linamat";
				Material*	   mat			= new Material(m_resourceManager, true, materialPath, TO_SID(materialPath));
				mat->SetShader(shaderSID);
				mat->SetProperty("color", Vector4::One);
				m_engineMaterials.push_back(mat);
			}
		}

		m_renderer->ResetResources();
		m_postInited = true;
	}

	void GfxManager::PreShutdown()
	{
		LinaVG::Terminate();

		for (auto m : m_engineMaterials)
			delete m;
		for (auto s : m_engineSamplers)
			delete s;
		for (auto sr : m_surfaceRenderers)
			delete sr;

		m_surfaceRenderers.clear();
	}

	void GfxManager::Shutdown()
	{
		m_meshManager.Shutdown();

		for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_dataPerFrame[i];
			m_renderer->DeleteCPUResource(data.globalDataBuffer);
		}

		m_renderer->Shutdown();
		delete m_renderer;
		m_resourceManager->RemoveListener(this);
	}

	void GfxManager::WaitForSwapchains()
	{
		m_renderer->WaitForSwapchains(m_surfaceRenderers[0]->GetSwapchain());
	}

	void GfxManager::Join()
	{
		m_renderer->Join();
	}

	void GfxManager::Tick(float interpolationAlpha)
	{
		PROFILER_FUNCTION();

		if (m_worldRenderers.size() == 1)
			m_worldRenderers[0]->Tick(interpolationAlpha);
		else
		{
			Taskflow tf;
			tf.for_each_index(0, static_cast<int>(m_worldRenderers.size()), 1, [&](int i) { m_worldRenderers[i]->Tick(interpolationAlpha); });
			m_system->GetMainExecutor()->RunAndWait(tf);
		}
	}

	void GfxManager::Sync()
	{
		PROFILER_FUNCTION();

		// Deletion of world renderers.
		{
			if (!m_worldRendererDeleteRequests.empty())
				Join();

			for (auto& req : m_worldRendererDeleteRequests)
			{
				m_worldRenderers.erase(linatl::find_if(m_worldRenderers.begin(), m_worldRenderers.end(), [&req](WorldRenderer* r) { return r == req.renderer; }));
				delete req.renderer;
				req.onDestroyed();
			}

			for (auto& req : m_worldRendererCreateRequests)
			{
				WorldRenderer* renderer = new WorldRenderer(this, req.mask, req.world, req.size, req.size.x / req.size.y);
				m_worldRenderers.push_back(renderer);
				req.onCreated(renderer);
			}

			m_worldRendererCreateRequests.clear();
			m_worldRendererDeleteRequests.clear();
		}

		if (m_worldRenderers.size() == 1)
			m_worldRenderers[0]->Sync();
		else
		{
			Taskflow tf;
			tf.for_each_index(0, static_cast<int>(m_worldRenderers.size()), 1, [&](int i) { m_worldRenderers[i]->Sync(); });
			m_system->GetMainExecutor()->RunAndWait(tf);
		}
	}

	void GfxManager::Render()
	{
		PROFILER_FUNCTION();

		if (!m_renderer->IsOK())
		{
			m_system->OnCriticalGfxError();
			return;
		}

		auto& frame = m_dataPerFrame[m_frameIndex];

		// Global data
		{
			m_globalData.screenSizeMousePos = Vector2::Zero;
			m_globalData.deltaElapsed		= Vector2(SystemInfo::GetDeltaTimeF(), SystemInfo::GetAppTimeF());
			frame.globalDataBuffer->BufferData(&m_globalData, sizeof(GPUGlobalData));
		}

		m_renderer->BeginFrame(m_frameIndex);
		LinaVG::StartFrame(static_cast<int>(m_surfaceRenderers.size()));
		m_guiBackend->BindTextures();

		// World renderers.
		{
			const int worldRenderersSz = static_cast<int>(m_worldRenderers.size());

			if (worldRenderersSz == 1)
				m_worldRenderers[0]->Render(m_frameIndex);
			else
			{
				Taskflow worldRendererTaskFlow;
				worldRendererTaskFlow.for_each_index(0, worldRenderersSz, 1, [&](int i) {
					auto wr = m_worldRenderers[i];
					wr->Render(m_frameIndex);
				});

				m_system->GetMainExecutor()->RunAndWait(worldRendererTaskFlow);
			}
		}

		// Surface renderers.
		{
			if (m_surfaceRenderers.size() == 1)
			{
				m_surfaceRenderers[0]->Render(0, m_frameIndex);
				m_surfaceRenderers[0]->Present();
			}
			else
			{
				Taskflow tf;
				tf.for_each_index(0, static_cast<int>(m_surfaceRenderers.size()), 1, [&](int i) {
					m_surfaceRenderers[i]->Render(i, m_frameIndex);
					m_surfaceRenderers[i]->Present();
				});

				m_system->GetMainExecutor()->RunAndWait(tf);
			}
		}

		LinaVG::EndFrame();
		m_renderer->EndFrame(m_frameIndex);
		m_frameIndex = (m_frameIndex + 1) % FRAMES_IN_FLIGHT;

		if (!m_renderer->IsOK())
		{
			m_system->OnCriticalGfxError();
			return;
		}
	}

	void GfxManager::CreateSurfaceRenderer(StringID sid, IWindow* window, const Vector2i& initialSize, Bitmask16 mask)
	{
		SurfaceRenderer* renderer = new SurfaceRenderer(this, BACK_BUFFER_COUNT, sid, window, initialSize, mask);
		m_surfaceRenderers.push_back(renderer);
	}

	void GfxManager::DestroySurfaceRenderer(StringID sid)
	{
		auto it = linatl::find_if(m_surfaceRenderers.begin(), m_surfaceRenderers.end(), [sid](SurfaceRenderer* renderer) { return renderer->GetSwapchain()->GetSID() == sid; });

		// Might already be deleted due to critical gfx error.
		if (it == m_surfaceRenderers.end())
			return;

		delete *it;
		m_surfaceRenderers.erase(it);
	}

	IGfxResourceCPU* GfxManager::GetCurrentGlobalDataResource()
	{
		return m_dataPerFrame[m_frameIndex].globalDataBuffer;
	}

	void GfxManager::OnSystemEvent(SystemEvent eventType, const Event& ev)
	{
		bool flushModels  = false;
		bool requireReset = false;

		if (eventType & EVS_ResourceLoadTaskCompleted)
		{
			ResourceLoadTask* task = static_cast<ResourceLoadTask*>(ev.pParams[0]);

			for (const auto& ident : task->identifiers)
			{
				if (!flushModels && ident.tid == GetTypeID<Model>())
					flushModels = true;

				if (!requireReset && (ident.tid == GetTypeID<Texture>() || ident.tid == GetTypeID<Font>() || ident.tid == GetTypeID<TextureSampler>()))
					requireReset = true;
			}

			if (flushModels)
				m_meshManager.MergeMeshes();

			if (requireReset)
				m_renderer->GetUploadContext()->MarkRequireReset();

			m_guiBackend->OnResourceBatchLoaded(ev);

			if (flushModels || requireReset)
				m_renderer->GetUploadContext()->Flush();
		}
		else if (eventType & EVS_LevelInstalled)
		{
		}
	}

	void GfxManager::OnWindowMoved(IWindow* window, StringID sid, const Recti& rect)
	{
	}

	void GfxManager::OnWindowResized(IWindow* window, StringID sid, const Recti& rect)
	{
		if (!m_postInited)
			return;

		m_renderer->OnWindowResized(window, sid, rect);
	}

	void GfxManager::OnVsyncChanged(VsyncMode mode)
	{
		if (!m_postInited)
			return;
		m_renderer->OnVsyncChanged(mode);
	}

	SurfaceRenderer* GfxManager::GetSurfaceRenderer(StringID sid)
	{
		auto it = linatl::find_if(m_surfaceRenderers.begin(), m_surfaceRenderers.end(), [sid](SurfaceRenderer* renderer) { return renderer->GetSwapchain()->GetSID() == sid; });
		return *it;
	}

	void GfxManager::CreateWorldRenderer(Delegate<void(WorldRenderer*)>&& onCreated, EntityWorld* world, const Vector2& size, WorldRendererMask mask)
	{
		CreateWorldRendererRequest req;
		req.onCreated = onCreated;
		req.mask	  = mask;
		req.size	  = size;
		req.world	  = world;

		m_worldRendererCreateRequests.push_back(req);
	}

	void GfxManager::DestroyWorldRenderer(Delegate<void()>&& onDestroyed, WorldRenderer* renderer, bool immediate)
	{
		if (immediate)
		{
			m_worldRenderers.erase(linatl::find_if(m_worldRenderers.begin(), m_worldRenderers.end(), [&renderer](WorldRenderer* r) { return r == renderer; }));
			delete renderer;
			return;
		}
		m_worldRendererDeleteRequests.push_back({renderer, onDestroyed});
	}

} // namespace Lina
