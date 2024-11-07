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

#include "Editor/Graphics/EditorRenderer.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Graphics/SurfaceRenderer.hpp"
#include "Editor/Graphics/EditorGfxHelpers.hpp"
#include "Common/Data/CommonData.hpp"

#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Resource/TextureSampler.hpp"
#include "Core/Graphics/Resource/Font.hpp"
#include "Core/Application.hpp"

namespace Lina::Editor
{
	void EditorRenderer::Initialize(Editor* editor)
	{
		m_editor = editor;
		m_guiBackend.Initialize(&m_editor->GetResourceManagerV2());
		m_lgx				= Application::GetLGX();
		m_resourceManagerV2 = &editor->GetResourceManagerV2();
		m_guiSampler		= m_resourceManagerV2->CreateResource<TextureSampler>(m_resourceManagerV2->ConsumeResourceID(), "EditorRendererGUISampler");
		m_guiTextSampler	= m_resourceManagerV2->CreateResource<TextureSampler>(m_resourceManagerV2->ConsumeResourceID(), "EditorRendererGUITextSampler");

		LinaGX::SamplerDesc samplerData = {};
		samplerData.minFilter			= LinaGX::Filter::Anisotropic;
		samplerData.magFilter			= LinaGX::Filter::Anisotropic;
		samplerData.mode				= LinaGX::SamplerAddressMode::Repeat;
		samplerData.anisotropy			= 6;
		samplerData.borderColor			= LinaGX::BorderColor::WhiteOpaque;
		samplerData.mipLodBias			= 0.0f;
		samplerData.minLod				= 0.0f;
		samplerData.maxLod				= 12;
		m_guiSampler->GenerateHW(samplerData);

		samplerData.magFilter = LinaGX::Filter::Anisotropic;
		m_guiTextSampler->GenerateHW(samplerData);

		m_pipelineLayoutGlobal = m_lgx->CreatePipelineLayout(EditorGfxHelpers::GetPipelineLayoutDescriptionGlobal());
		m_pipelineLayoutGUI	   = m_lgx->CreatePipelineLayout(EditorGfxHelpers::GetPipelineLayoutDescriptionGUI());

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];

			data.descriptorSetGlobal = m_lgx->CreateDescriptorSet(EditorGfxHelpers::GetSetDescriptionGlobal());

			data.globalTexturesDesc = {
				.setHandle = data.descriptorSetGlobal,
				.binding   = 0,
			};
			data.globalSamplersDesc = {
				.setHandle = data.descriptorSetGlobal,
				.binding   = 1,
			};

			data.copyStream	   = m_lgx->CreateCommandStream({LinaGX::CommandType::Transfer, 50, 12000, 4096, 32, "WorldRenderer: Copy Stream"});
			data.copySemaphore = SemaphoreData(m_lgx->CreateUserSemaphore());
		}
	}

	void EditorRenderer::Shutdown()
	{
		m_resourceManagerV2->DestroyResource(m_guiSampler);
		m_resourceManagerV2->DestroyResource(m_guiTextSampler);

		m_lgx->DestroyPipelineLayout(m_pipelineLayoutGlobal);
		m_lgx->DestroyPipelineLayout(m_pipelineLayoutGUI);

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			auto& data = m_pfd[i];
			m_lgx->DestroyDescriptorSet(data.descriptorSetGlobal);
			m_lgx->DestroyCommandStream(data.copyStream);
			m_lgx->DestroyUserSemaphore(data.copySemaphore.GetSemaphore());
		}

		m_guiBackend.Shutdown();
	}

	void EditorRenderer::PreTick()
	{
		m_validSurfaceRenderers.resize(0);

		// Main thread required.
		for (SurfaceRenderer* rend : m_surfaceRenderers)
		{
			if (rend->CheckVisibility())
				m_validSurfaceRenderers.push_back(rend);

			rend->PreTick();
		}

		/*
		if (m_validSurfaceRenderers.size() == 1)
		{
			m_validSurfaceRenderers[0]->PreTick();
		}
		else
		{
			Taskflow tf;
			tf.for_each_index(0, static_cast<int>(m_validSurfaceRenderers.size()), 1, [&](int i) {
				SurfaceRenderer* rend = m_validSurfaceRenderers.at(i);
				rend->PreTick();
			});
			m_editor->GetExecutor().RunAndWait(tf);
		}
		*/
	}
	void EditorRenderer::Tick(float delta)
	{
		for (WorldRenderer* rend : m_worldRenderers)
			rend->Tick(delta);

		if (m_validSurfaceRenderers.size() == 1)
		{
			m_validSurfaceRenderers[0]->Tick(delta);
		}
		else
		{
			Taskflow tf;
			tf.for_each_index(0, static_cast<int>(m_validSurfaceRenderers.size()), 1, [&](int i) {
				SurfaceRenderer* rend = m_validSurfaceRenderers.at(i);
				rend->Tick(delta);
			});
			m_editor->GetExecutor().RunAndWait(tf);
		}
	}

	void EditorRenderer::UpdateBindlessResources(uint32 frameIndex)
	{
		auto& pfd = m_pfd[frameIndex];

		if (!pfd.bindlessDirty)
			return;

		// Textures.
		ResourceCache<Texture>* cacheTxt	 = m_resourceManagerV2->GetCache<Texture>();
		const size_t			txtCacheSize = static_cast<size_t>(cacheTxt->GetActiveItemCount());
		pfd.globalTexturesDesc.textures.resize(txtCacheSize + m_dynamicTextures.size());
		cacheTxt->View([&](Texture* txt, uint32 index) -> bool {
			pfd.globalTexturesDesc.textures[index] = txt->GetGPUHandle();
			SetBindlessIndex(txt, static_cast<uint32>(index));
			return false;
		});

		for (size_t i = 0; i < m_dynamicTextures.size(); i++)
		{
			Texture* txt									  = m_dynamicTextures.at(i);
			pfd.globalTexturesDesc.textures[txtCacheSize + i] = txt->GetGPUHandle();
			SetBindlessIndex(txt, static_cast<uint32>(i + txtCacheSize));
		}

		if (!pfd.globalTexturesDesc.textures.empty())
			m_lgx->DescriptorUpdateImage(pfd.globalTexturesDesc);

		// Samplers
		ResourceCache<TextureSampler>* cacheSmp = m_resourceManagerV2->GetCache<TextureSampler>();
		pfd.globalSamplersDesc.samplers.resize(static_cast<size_t>(cacheSmp->GetActiveItemCount()));
		cacheSmp->View([&](TextureSampler* smp, uint32 index) -> bool {
			pfd.globalSamplersDesc.samplers[index] = smp->GetGPUHandle();
			SetBindlessIndex(smp, static_cast<uint32>(index));
			return false;
		});

		if (!pfd.globalSamplersDesc.samplers.empty())
			m_lgx->DescriptorUpdateImage(pfd.globalSamplersDesc);

		pfd.bindlessDirty = false;
	}

	void EditorRenderer::SyncRender()
	{
		for (SurfaceRenderer* sr : m_validSurfaceRenderers)
			sr->SyncRender(m_lgx->GetCurrentFrameIndex());
	}

	void EditorRenderer::Render()
	{
		const uint32 frameIndex = m_lgx->GetCurrentFrameIndex();

		m_lgx->StartFrame();

		auto& pfd = m_pfd[frameIndex];

		Vector<WorldRenderer*> validWorlds;
		validWorlds.resize(m_worldRenderers.size());

		Taskflow tf;
		tf.for_each_index(0, static_cast<int>(m_worldRenderers.size()), 1, [&](int i) {
			WorldRenderer* rend = m_worldRenderers.at(i);
			rend->Render(frameIndex);
			validWorlds[i] = rend;
		});
		m_editor->GetExecutor().RunAndWait(tf);

		UpdateBindlessResources(frameIndex);
		if (m_uploadQueue.FlushAll(pfd.copyStream))
		{
			BumpAndSendTransfers(frameIndex);
			m_lgx->WaitForUserSemaphore(pfd.copySemaphore.GetSemaphore(), pfd.copySemaphore.GetValue());
		}

		Vector<uint16> waitSemaphores;
		Vector<uint64> waitValues;

		for (WorldRenderer* wr : validWorlds)
		{
			const SemaphoreData sem = wr->GetSubmitSemaphore(frameIndex);
			waitSemaphores.push_back(sem.GetSemaphore());
			waitValues.push_back(sem.GetValue());
		}

		Vector<LinaGX::CommandStream*> streams;
		Vector<uint8>				   swapchains;
		streams.resize(m_validSurfaceRenderers.size());
		swapchains.resize(m_validSurfaceRenderers.size());

		if (m_validSurfaceRenderers.size() == 1)
		{
			SurfaceRenderer* rend = m_validSurfaceRenderers[0];
			streams[0]			  = rend->Render(frameIndex);
			swapchains[0]		  = rend->GetSwapchain();
		}
		else
		{
			Taskflow tf;
			tf.for_each_index(0, static_cast<int>(m_validSurfaceRenderers.size()), 1, [&](int i) {
				SurfaceRenderer* rend = m_validSurfaceRenderers.at(i);
				streams[i]			  = rend->Render(frameIndex);
				swapchains[i]		  = rend->GetSwapchain();
			});
			m_editor->GetExecutor().RunAndWait(tf);
		}

		for (SurfaceRenderer* rend : m_validSurfaceRenderers)
		{
			const SemaphoreData& sem = rend->GetWaitSemaphore(frameIndex);
			if (sem.IsModified())
			{
				waitValues.push_back(sem.GetValue());
				waitSemaphores.push_back(sem.GetSemaphore());
			}
		}

		if (streams.empty())
			return;

		m_lgx->SubmitCommandStreams({
			.targetQueue	= m_lgx->GetPrimaryQueue(LinaGX::CommandType::Graphics),
			.streams		= streams.data(),
			.streamCount	= static_cast<uint32>(streams.size()),
			.useWait		= !waitValues.empty(),
			.waitCount		= static_cast<uint32>(waitValues.size()),
			.waitSemaphores = waitSemaphores.data(),
			.waitValues		= waitValues.data(),
			.useSignal		= false,
		});

		m_lgx->Present({
			.swapchains		= swapchains.data(),
			.swapchainCount = static_cast<uint32>(swapchains.size()),
		});

		m_lgx->EndFrame();
	}

	void EditorRenderer::AddWorldRenderer(WorldRenderer* wr)
	{
		m_worldRenderers.push_back(wr);
		RefreshDynamicTextures();
	}

	void EditorRenderer::RemoveWorldRenderer(WorldRenderer* wr)
	{
		m_worldRenderers.erase(linatl::find_if(m_worldRenderers.begin(), m_worldRenderers.end(), [wr](WorldRenderer* rend) -> bool { return wr == rend; }));
		RefreshDynamicTextures();
	}

	void EditorRenderer::AddSurfaceRenderer(SurfaceRenderer* sr)
	{
		m_surfaceRenderers.push_back(sr);
	}

	void EditorRenderer::RemoveSurfaceRenderer(SurfaceRenderer* sr)
	{
		m_surfaceRenderers.erase(linatl::find_if(m_surfaceRenderers.begin(), m_surfaceRenderers.end(), [sr](SurfaceRenderer* rend) -> bool { return sr == rend; }));
	}

	void EditorRenderer::RefreshDynamicTextures()
	{
		m_dynamicTextures.clear();

		for (WorldRenderer* wr : m_worldRenderers)
		{
			for (int32 i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				m_dynamicTextures.push_back(wr->GetGBufAlbedo(i));
				m_dynamicTextures.push_back(wr->GetGBufPosition(i));
				m_dynamicTextures.push_back(wr->GetGBufNormal(i));
				m_dynamicTextures.push_back(wr->GetGBufDepth(i));
				m_dynamicTextures.push_back(wr->GetLightingPassOutput(i));
			}
		}

		MarkBindlessDirty();
	}

	void EditorRenderer::VerifyResources()
	{
		Application::GetLGX()->Join();

		ResourceCache<Texture>*		   textureCache = m_resourceManagerV2->GetCache<Texture>();
		ResourceCache<TextureSampler>* samplerCache = m_resourceManagerV2->GetCache<TextureSampler>();
		ResourceCache<Shader>*		   shaderCache	= m_resourceManagerV2->GetCache<Shader>();
		ResourceCache<Font>*		   fontCache	= m_resourceManagerV2->GetCache<Font>();

		bool bindlessDirty = false;

		textureCache->View([&](Texture* res, uint32 index) -> bool {
			if (res->GetIsReloaded())
			{
				res->DestroyHW();
				res->SetIsReloaded(false);
			}

			if (!res->IsHWValid())
			{
				res->GenerateHW();
				res->AddToUploadQueue(m_uploadQueue, false);
				bindlessDirty = true;
			}
			return false;
		});

		samplerCache->View([&](TextureSampler* res, uint32 index) -> bool {
			if (res->GetIsReloaded())
			{
				res->DestroyHW();
				res->SetIsReloaded(false);
			}

			if (!res->IsHWValid())
			{
				res->GenerateHW();
				bindlessDirty = true;
			}

			return false;
		});

		shaderCache->View([&](Shader* res, uint32 index) -> bool {
			if (res->GetIsReloaded())
			{
				res->DestroyHW();
				res->SetIsReloaded(false);
			}

			if (!res->IsHWValid())
				res->GenerateHW();
			return false;
		});

		bool containsFont = false;
		fontCache->View([&](Font* res, uint32 index) -> bool {
			if (res->GetIsReloaded())
			{
				res->DestroyHW();
				res->SetIsReloaded(false);
			}

			if (!res->IsHWValid())
			{
				res->GenerateHW();
				res->Upload(m_guiBackend.GetLVGText());
				containsFont = true;
			}

			return false;
		});

		if (containsFont)
			m_guiBackend.ReuploadAtlases(m_uploadQueue);

		if (bindlessDirty)
			MarkBindlessDirty();
	}

	void EditorRenderer::BumpAndSendTransfers(uint32 frameIndex)
	{
		auto& pfd = m_pfd[frameIndex];
		pfd.copySemaphore.Increment();
		m_lgx->CloseCommandStreams(&pfd.copyStream, 1);
		m_lgx->SubmitCommandStreams({
			.targetQueue	  = m_lgx->GetPrimaryQueue(LinaGX::CommandType::Transfer),
			.streams		  = &pfd.copyStream,
			.streamCount	  = 1,
			.useSignal		  = true,
			.signalCount	  = 1,
			.signalSemaphores = pfd.copySemaphore.GetSemaphorePtr(),
			.signalValues	  = pfd.copySemaphore.GetValuePtr(),
		});
	}
} // namespace Lina::Editor
