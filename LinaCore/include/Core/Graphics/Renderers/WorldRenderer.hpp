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

#pragma once

#include "Core/Graphics/Pipeline/Buffer.hpp"
#include "Core/Graphics/CommonGraphics.hpp"
#include "Core/Graphics/Pipeline/RenderPass.hpp"
#include "Core/Graphics/ResourceUploadQueue.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/Graphics/MeshManager.hpp"
#include "Core/Graphics/GUI/GUIBackend.hpp"
#include "Core/Graphics/Renderers/SkyRenderer.hpp"
#include "Core/Graphics/Renderers/LightingRenderer.hpp"
#include "Core/Graphics/Renderers/DrawCollector.hpp"

namespace LinaGX
{
	class CommandStream;
	class Instance;
	struct Camera;
	class Screen;
} // namespace LinaGX

namespace Lina
{
	class CompModel;
	class Shader;
	class Texture;
	class ResourceManagerV2;
	class WorldRenderer;
	class FeatureRenderer;

	class WorldRendererExtension
	{
	public:
		virtual ~WorldRendererExtension() = default;

	protected:
		friend class WorldRenderer;

		virtual void Tick(float delta){};
		virtual void RenderForward(uint32 frameIndex, LinaGX::CommandStream* stream){};
		virtual void AddBuffersToUploadQueue(uint32 frameIndex, ResourceUploadQueue& queue){};

		WorldRenderer* m_worldRenderer = nullptr;
	};

	class WorldRenderer : public EntityWorldListener
	{
	private:
		struct PerFrameData
		{
			LinaGX::CommandStream* gfxStream	   = nullptr;
			LinaGX::CommandStream* copyStream	   = nullptr;
			SemaphoreData		   copySemaphore   = {};
			SemaphoreData		   signalSemaphore = {};

			Buffer entityDataBuffer;
			Buffer boneBuffer;
			Buffer instanceDataBuffer;

			Texture* gBufAlbedo			= nullptr;
			Texture* gBufPosition		= nullptr;
			Texture* gBufNormal			= nullptr;
			Texture* gBufDepth			= nullptr;
			Texture* lightingPassOutput = nullptr;
		};

		struct RenderData
		{
		};

	public:
		WorldRenderer(GfxContext* context, ResourceManagerV2* rm, EntityWorld* world, const Vector2ui& viewSize, const String& name = "", Buffer* snapshotBuffer = nullptr, bool standaloneSubmit = false);
		~WorldRenderer();

		void Tick(float delta);
		void Render(uint32 frameIndex);
		void Resize(const Vector2ui& newSize);
		void SyncRender();
		void DropRenderFrame();

		virtual void OnComponentAdded(Component* c) override;
		virtual void OnComponentRemoved(Component* c) override;
		void		 AddFeatureRenderer(FeatureRenderer* ft);
		void		 RemoveFeatureRenderer(FeatureRenderer* ft);

		inline Buffer& GetInstanceDataBuffer(uint32 frameIndex)
		{
			return m_pfd[frameIndex].instanceDataBuffer;
		}

		inline Buffer& GetEntityDataBuffer(uint32 frameIndex)
		{
			return m_pfd[frameIndex].entityDataBuffer;
		}

		inline Buffer& GetBoneBuffer(uint32 frameIndex)
		{
			return m_pfd[frameIndex].boneBuffer;
		}

		inline SemaphoreData GetSubmitSemaphore(uint32 frameIndex)
		{
			if (m_snapshotBuffer != nullptr)
				return m_pfd[frameIndex].copySemaphore;

			return m_pfd[frameIndex].signalSemaphore;
		};

		inline EntityWorld* GetWorld() const
		{
			return m_world;
		}

		inline const Vector2ui& GetSize() const
		{
			return m_size;
		}

		inline Texture* GetGBufAlbedo(uint32 frameIndex)
		{
			return m_pfd[frameIndex].gBufAlbedo;
		}

		inline Texture* GetGBufPosition(uint32 frameIndex)
		{
			return m_pfd[frameIndex].gBufPosition;
		}

		inline Texture* GetGBufNormal(uint32 frameIndex)
		{
			return m_pfd[frameIndex].gBufNormal;
		}

		inline Texture* GetGBufDepth(uint32 frameIndex)
		{
			return m_pfd[frameIndex].gBufDepth;
		}

		inline Texture* GetLightingPassOutput(uint32 frameIndex)
		{
			return m_pfd[frameIndex].lightingPassOutput;
		}

		inline Buffer* GetSnapshotBuffer() const
		{
			return m_snapshotBuffer;
		}

		inline const String& GetName() const
		{
			return m_name;
		}

		inline void SetExtension(WorldRendererExtension* ext)
		{
			m_extension = ext;
		}

		inline DrawCollector& GetDrawCollector()
		{
			return m_drawCollector;
		}

		inline SemaphoreData& GetSignalSemaphoreData(uint32 frameIndex)
		{
			return m_pfd[frameIndex].signalSemaphore;
		}

		inline SemaphoreData& GetCopySemaphoreData(uint32 frameIndex)
		{
			return m_pfd[frameIndex].copySemaphore;
		}

	private:
		void   UpdateBuffers(uint32 frameIndex);
		void   CreateSizeRelativeResources();
		void   DestroySizeRelativeResources();
		uint64 BumpAndSendTransfers(uint32 frameIndex);

	private:
		GUIBackend				 m_guiBackend;
		ResourceManagerV2*		 m_resourceManagerV2 = nullptr;
		LinaGX::Instance*		 m_lgx				 = nullptr;
		ResourceUploadQueue		 m_uploadQueue;
		PerFrameData			 m_pfd[FRAMES_IN_FLIGHT]  = {};
		RenderPass				 m_deferredPass			  = {};
		RenderPass				 m_forwardPass			  = {};
		Vector2ui				 m_size					  = Vector2ui::Zero;
		EntityWorld*			 m_world				  = nullptr;
		TextureSampler*			 m_gBufSampler			  = nullptr;
		Shader*					 m_deferredLightingShader = nullptr;
		Buffer*					 m_snapshotBuffer		  = nullptr;
		bool					 m_standaloneSubmit		  = false;
		Vector<FeatureRenderer*> m_featureRenderers;
		GfxContext*				 m_gfxContext;
		String					 m_name		 = "";
		WorldRendererExtension*	 m_extension = nullptr;
		SkyRenderer				 m_skyRenderer;
		LightingRenderer		 m_lightingRenderer;
		JobExecutor				 m_executor;
		DrawCollector			 m_drawCollector;
	};

} // namespace Lina
