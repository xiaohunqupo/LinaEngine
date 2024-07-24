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

#include "Core/Graphics/GfxManager.hpp"
#include "Core/Application.hpp"
#include "Common/System/System.hpp"
#include "Common/Profiling/Profiler.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Core/ApplicationDelegate.hpp"

namespace
{
	LINAGX_STRING FormatString(const char* fmt, va_list args)
	{
		// Determine the required size
		va_list args_copy;
		va_copy(args_copy, args);
		int size = vsnprintf(nullptr, 0, fmt, args_copy) + 1; // +1 for the null terminator
		va_end(args_copy);

		// Allocate a buffer and format the string
		std::vector<char> buffer(size);
		vsnprintf(buffer.data(), size, fmt, args);

		return std::string(buffer.data());
	}

	void LinaGX_ErrorCallback(const char* err, ...)
	{
		va_list args;
		va_start(args, err);
		std::string formattedStr = FormatString(err, args);
		LINA_ERR(formattedStr.c_str());
		va_end(args);
	}

	void LinaGX_LogCallback(const char* err, ...)
	{
		va_list args;
		va_start(args, err);
		std::string formattedStr = FormatString(err, args);
		LINA_INFO(formattedStr.c_str());
		va_end(args);
	}
} // namespace

namespace Lina
{

	LinaGX::Instance* GfxManager::s_lgx = nullptr;

	GfxManager::GfxManager(System* sys) : Subsystem(sys, SubsystemType::GfxManager)
	{
		// Setup LinaVG
		LinaVG::Config.globalFramebufferScale = 1.0f;
		LinaVG::Config.globalAAMultiplier	  = 1.0f;
		LinaVG::Config.gcCollectInterval	  = 4000;
		LinaVG::Config.textCachingEnabled	  = true;
		LinaVG::Config.textCachingSDFEnabled  = true;
		LinaVG::Config.textCacheReserve		  = 10000;
		LinaVG::Config.maxFontAtlasSize		  = 1024;
		LinaVG::Config.errorCallback		  = [](const std::string& err) { LINA_ERR(err.c_str()); };
		LinaVG::Config.logCallback			  = [](const std::string& log) { LINA_TRACE(log.c_str()); };

		LinaVG::InitializeText();
	}

	void GfxManager::PreInitialize(const SystemInitializationInfo& initInfo)
	{
		// Setup LinaGX
		m_lgx		  = new LinaGX::Instance();
		m_appDelegate = m_system->GetApp()->GetAppDelegate();
		s_lgx		  = m_lgx;

		LinaGX::Config.dx12Config = {
			.allowTearing				 = initInfo.allowTearing,
			.enableDebugLayers			 = true,
			.serializeShaderDebugSymbols = true,
		};

		LinaGX::Config.vulkanConfig = {
			.enableVulkanFeatures = LinaGX::VulkanFeatureFlags::VKF_Bindless | LinaGX::VulkanFeatureFlags::VKF_MultiDrawIndirect,
		};

		LinaGX::Config.gpuLimits = {
			.samplerLimit = 2048,
			.bufferLimit  = 2048,
		};

		LinaGX::Config.logLevel		 = LinaGX::LogLevel::Verbose;
		LinaGX::Config.errorCallback = LinaGX_ErrorCallback;
		LinaGX::Config.infoCallback	 = LinaGX_LogCallback;
		LinaGX::BackendAPI api		 = LinaGX::BackendAPI::DX12;

#ifdef LINA_PLATFORM_APPLE
		api = LinaGX::BackendAPI::Metal;
#endif

		LinaGX::Config.multithreadedQueueSubmission = true;
		LinaGX::Config.api							= api;
		LinaGX::Config.gpu							= LinaGX::PreferredGPUType::Discrete;
		LinaGX::Config.framesInFlight				= FRAMES_IN_FLIGHT;
		LinaGX::Config.backbufferCount				= BACK_BUFFER_COUNT;
		LinaGX::Config.mutexLockCreationDeletion	= true;

		m_lgx->Initialize();

		// Default samplers
		/*
		{
			LinaGX::SamplerDesc samplerData = {};
			samplerData.minFilter			= LinaGX::Filter::Anisotropic;
			samplerData.magFilter			= LinaGX::Filter::Anisotropic;
			samplerData.mode				= LinaGX::SamplerAddressMode::Repeat;
			samplerData.anisotropy			= 6;
			samplerData.borderColor			= LinaGX::BorderColor::WhiteOpaque;
			samplerData.mipLodBias			= 0.0f;
			samplerData.minLod				= 0.0f;
			samplerData.maxLod				= 30.0f; // upper limit

			TextureSampler* defaultSampler		  = m_resourceManager->CreateResource<TextureSampler>(DEFAULT_SAMPLER_PATH, DEFAULT_SAMPLER_SID);
			TextureSampler* defaultGUISampler	  = m_resourceManager->CreateResource<TextureSampler>(DEFAULT_SAMPLER_GUI_PATH, DEFAULT_SAMPLER_GUI_SID);
			TextureSampler* defaultGUITextSampler = m_resourceManager->CreateResource<TextureSampler>(DEFAULT_SAMPLER_TEXT_PATH, DEFAULT_SAMPLER_TEXT_SID);
			defaultSampler->m_samplerDesc		  = samplerData;
			defaultSampler->GenerateHW();

			samplerData.mipLodBias			 = -1.0f;
			samplerData.mode				 = LinaGX::SamplerAddressMode::ClampToEdge;
			samplerData.magFilter			 = LinaGX::Filter::Linear;
			defaultGUISampler->m_samplerDesc = samplerData;
			defaultGUISampler->GenerateHW();

			samplerData.minFilter				 = LinaGX::Filter::Anisotropic;
			samplerData.magFilter				 = LinaGX::Filter::Anisotropic;
			samplerData.mode					 = LinaGX::SamplerAddressMode::ClampToEdge;
			samplerData.mipLodBias				 = 0.0f;
			defaultGUITextSampler->m_samplerDesc = samplerData;
			defaultGUITextSampler->GenerateHW();

			m_defaultSamplers.push_back(defaultSampler);
			m_defaultSamplers.push_back(defaultGUISampler);
			m_defaultSamplers.push_back(defaultGUITextSampler);
		}*/
	}

	void GfxManager::Initialize(const SystemInitializationInfo& initInfo)
	{
		// Default materials
		/*
		{

			Material* defaultObjectMaterial = m_resourceManager->CreateResource<Material>(DEFAULT_MATERIAL_OBJECT_PATH, DEFAULT_MATERIAL_OBJECT_SID);
			defaultObjectMaterial->SetShader(DEFAULT_SHADER_OBJECT_SID);
			defaultObjectMaterial->SetProperty("albedo"_hs, LinaTexture2D{DEFAULT_TEXTURE_CHECKERED_DARK_SID, m_defaultSamplers[0]->GetSID()});
			m_defaultMaterials.push_back(defaultObjectMaterial);

			Material* defaultSkyMaterial = m_resourceManager->CreateResource<Material>(DEFAULT_MATERIAL_SKY_PATH, DEFAULT_MATERIAL_SKY_SID);
			defaultSkyMaterial->SetShader(DEFAULT_SHADER_SKY_SID);
			m_defaultMaterials.push_back(defaultSkyMaterial);

			// Material* defaultUnlitMaterial = new Material(m_resourceManager, true, "Resources/Core/Materials/DefaultUnlit.linamaterial", DEFAULT_UNLIT_MATERIAL);
			// Material* defaultLitMaterial   = new Material(m_resourceManager, true, "Resources/Core/Materials/DefaultLit.linamaterial", DEFAULT_LIT_MATERIAL);
			// defaultLitMaterial->SetShader("Resources/Core/Shaders/LitStandard.linashader"_hs);
			// defaultUnlitMaterial->SetShader("Resources/Core/Shaders/UnlitStandard.linashader"_hs);
			// m_defaultMaterials.push_back(defaultLitMaterial);
			// m_defaultMaterials.push_back(defaultUnlitMaterial);
		}
		 */
	}

	void GfxManager::PreShutdown()
	{
		// Preshutdown is before resource manager, make sure
		// to remove user managed resources.
		LinaVG::TerminateText();
	}

	void GfxManager::Shutdown()
	{
		DestroyApplicationWindow(LINA_MAIN_SWAPCHAIN);
		delete m_lgx;
	}

	void GfxManager::WaitForSwapchains()
	{
	}

	void GfxManager::Join()
	{
		m_lgx->Join();
	}

	void GfxManager::Poll()
	{
		m_lgx->TickWindowSystem();
	}

	void GfxManager::PreTick()
	{
		WaitForSwapchains();
	}

	void GfxManager::Tick(float delta)
	{
		PROFILER_FUNCTION();
	}

	void GfxManager::OnWindowSizeChanged(LinaGX::Window* window, const LinaGX::LGXVector2ui& sz)
	{
		m_appDelegate->OnWindowSizeChanged(window, sz);
	}

	void GfxManager::Render(StringID targetPool)
	{
		PROFILER_FUNCTION();
		const uint32 currentFrameIndex = m_lgx->GetCurrentFrameIndex();

		m_lgx->StartFrame();

		m_appDelegate->Render(currentFrameIndex);

		/*
				if (m_mainWindow == nullptr)
					m_mainWindow = m_lgx->GetWindowManager().GetWindow(LINA_MAIN_SWAPCHAIN);

				// Update global data.
				{
					const auto&			 mp			= m_lgx->GetInput().GetMousePositionAbs();
					const auto&			 windowSize = m_mainWindow->GetSize();
					GPUDataEngineGlobals globalData = {};
					globalData.mouseScreen			= Vector4(static_cast<float>(mp.x), static_cast<float>(mp.y), static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
					globalData.deltaElapsed			= Vector4(SystemInfo::GetDeltaTimeF(), SystemInfo::GetAppTimeF(), 0.0f, 0.0f);
					currentFrame.globalDataBuffer.BufferData(0, (uint8*)&globalData, sizeof(GPUDataEngineGlobals));
				}

				// Bindless resources
				{
					UpdateBindlessResources(currentFrame);
				}

				// Update dirty materials.
				{
					m_resourceUploadQueue.AddBufferRequest(&currentFrame.globalDataBuffer);
					m_resourceUploadQueue.AddBufferRequest(&currentFrame.globalMaterialsBuffer);
				}

				// Mesh manager refresh
				{
					m_meshManager.Refresh();
				}

				// Wait for all transfers.
				{

					if (m_resourceUploadQueue.FlushAll(currentFrame.globalCopyStream))
					{
						m_lgx->CloseCommandStreams(&currentFrame.globalCopyStream, 1);
						currentFrame.globalCopySemaphore.Increment();
						LinaGX::SubmitDesc desc = LinaGX::SubmitDesc{
							.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Transfer),
							.streams		  = &currentFrame.globalCopyStream,
							.streamCount	  = 1,
							.useSignal		  = true,
							.signalCount	  = 1,
							.signalSemaphores = currentFrame.globalCopySemaphore.GetSemaphorePtr(),
							.signalValues	  = currentFrame.globalCopySemaphore.GetValuePtr(),
							.isMultithreaded  = true,
						};
						m_lgx->SubmitCommandStreams(desc);
						m_lgx->WaitForUserSemaphore(currentFrame.globalCopySemaphore.GetSemaphore(), currentFrame.globalCopySemaphore.GetValue());
					}
				}

				// Renderer work
				Vector<uint16> waitSemaphores;
				Vector<uint64> waitValues;

				for (const RendererPool& pool : m_rendererPools)
				{
					if (targetPool != 0 && pool.sid != targetPool)
						continue;

					if (pool.renderers.empty())
						continue;

					Vector<Renderer*> validRenderers;

					for (auto* rend : pool.renderers)
					{
						if (rend->IsValidThisFrame())
							validRenderers.push_back(rend);
					}

					Vector<LinaGX::CommandStream*> streams;
					streams.resize(validRenderers.size());

					if (validRenderers.size() == 1)
					{
						auto* rend = validRenderers[0];
						rend->Render(currentFrameIndex, static_cast<uint32>(waitSemaphores.size()), waitSemaphores.data(), waitValues.data());
						streams[0] = rend->GetStreamForBatchSubmit(currentFrameIndex);
					}
					else
					{
						Taskflow tf;
						tf.for_each_index(0, static_cast<int>(validRenderers.size()), 1, [&](int i) {
							auto* rend = validRenderers.at(i);
							rend->Render(currentFrameIndex, static_cast<uint32>(waitSemaphores.size()), waitSemaphores.data(), waitValues.data());
							streams[i] = rend->GetStreamForBatchSubmit(currentFrameIndex);
						});
						m_system->GetMainExecutor()->RunAndWait(tf);
					}

					if (pool.submitInBatch)
					{

						for (Renderer* rend : pool.renderers)
						{
							const SemaphoreData waitSemaphore = rend->GetWaitSemaphore(currentFrameIndex);

							if (waitSemaphore.IsModified())
							{
								waitSemaphores.push_back(waitSemaphore.GetSemaphore());
								waitValues.push_back(waitSemaphore.GetValue());
							}
						}

						currentFrame.poolSubmissionSemaphore.Increment();

						m_lgx->SubmitCommandStreams({
							.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Graphics),
							.streams		  = streams.data(),
							.streamCount	  = static_cast<uint32>(streams.size()),
							.useWait		  = !waitValues.empty(),
							.waitCount		  = static_cast<uint32>(waitValues.size()),
							.waitSemaphores	  = waitSemaphores.data(),
							.waitValues		  = waitValues.data(),
							.useSignal		  = true,
							.signalSemaphores = currentFrame.poolSubmissionSemaphore.GetSemaphorePtr(),
							.signalValues	  = currentFrame.poolSubmissionSemaphore.GetValuePtr(),
							.isMultithreaded  = true,
						});

						waitValues.clear();
						waitSemaphores.clear();
						waitSemaphores.push_back(currentFrame.poolSubmissionSemaphore.GetSemaphore());
						waitValues.push_back(currentFrame.poolSubmissionSemaphore.GetValue());
					}
					else
					{
						waitValues.clear();
						waitSemaphores.clear();

						for (Renderer* rend : pool.renderers)
						{
							SemaphoreData submittedSemaphore = rend->GetSubmitSemaphore(currentFrameIndex);

							if (submittedSemaphore.IsModified())
							{
								waitSemaphores.push_back(submittedSemaphore.GetSemaphore());
								waitValues.push_back(submittedSemaphore.GetValue());
							}
						}
					}
				}

				// Presentation
				Vector<uint8> swapchains;

				for (const RendererPool& pool : m_rendererPools)
				{
					for (Renderer* rend : pool.renderers)
					{
						if (!rend->IsValidThisFrame())
							continue;

						uint8 outSwapchain = 0;
						if (rend->GetSwapchainToPresent(outSwapchain))
							swapchains.push_back(outSwapchain);
					}
				}

				if (!swapchains.empty())
				{
					m_lgx->Present({
						.swapchains		= swapchains.data(),
						.swapchainCount = static_cast<uint32>(swapchains.size()),
					});
				}
		*/
		m_lgx->EndFrame();
	}

	LinaGX::Window* GfxManager::CreateApplicationWindow(StringID sid, const char* title, const Vector2i& pos, const Vector2ui& size, uint32 style, LinaGX::Window* parentWindow)
	{
		m_lgx->Join();
		auto window = m_lgx->GetWindowManager().CreateApplicationWindow(sid, title, pos.x, pos.y, size.x, size.y, static_cast<LinaGX::WindowStyle>(style), parentWindow);
		window->AddListener(this);
		return window;
	}

	void GfxManager::DestroyApplicationWindow(StringID sid)
	{
		m_lgx->Join();
		auto* window = m_lgx->GetWindowManager().GetWindow(sid);
		window->RemoveListener(this);
		m_lgx->GetWindowManager().DestroyApplicationWindow(sid);
	}

	LinaGX::Window* GfxManager::GetApplicationWindow(StringID sid)
	{
		return m_lgx->GetWindowManager().GetWindow(sid);
	}
} // namespace Lina
