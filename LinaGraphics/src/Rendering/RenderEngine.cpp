/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: RenderEngine
Timestamp: 4/27/2019 11:18:07 PM

*/

#include "Rendering/RenderEngine.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/RenderConstants.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/ArrayBitmap.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/ECS.hpp"
#include "Utility/Math/Color.hpp"
#include "Utility/UtilityFunctions.hpp"
#include "Core/Layer.hpp"



namespace LinaEngine::Graphics
{

	constexpr size_t UNIFORMBUFFER_VIEWDATA_SIZE = sizeof(Matrix) * 2 + (sizeof(Vector4F) * 4);
	constexpr int UNIFORMBUFFER_VIEWDATA_BINDPOINT = 0;
	constexpr auto UNIFORMBUFFER_VIEWDATA_NAME = "ViewData";

	constexpr size_t UNIFORMBUFFER_LIGHTDATA_SIZE = (sizeof(int) * 8);
	constexpr int UNIFORMBUFFER_LIGHTDATA_BINDPOINT = 1;
	constexpr auto UNIFORMBUFFER_LIGHTDATA_NAME = "LightData";

	constexpr size_t UNIFORMBUFFER_DEBUGDATA_SIZE = (sizeof(bool) * 1);
	constexpr int UNIFORMBUFFER_DEBUGDATA_BINDPOINT = 2;
	constexpr auto UNIFORMBUFFER_DEBUGDATA_NAME = "DebugData";

	RenderEngine::RenderEngine()
	{
		LINA_CORE_TRACE("[Constructor] -> RenderEngine ({0})", typeid(*this).name());
	}

	RenderEngine::~RenderEngine()
	{

		// Dump the remaining memory.
		DumpMemory();

		// Release Vertex Array Objects
		m_SkyboxVAO = m_RenderDevice.ReleaseVertexArray(m_SkyboxVAO);

		LINA_CORE_TRACE("[Destructor] -> RenderEngine ({0})", typeid(*this).name());
	}

	void RenderEngine::Initialize(LinaEngine::ECS::ECSRegistry& ecsReg)
	{
		// Initialize the render device.
		m_RenderDevice.Initialize(m_LightingSystem, m_MainWindow.GetWidth(), m_MainWindow.GetHeight());

		// Construct the uniform buffer for global matrices.
		m_GlobalDataBuffer.Construct(m_RenderDevice, UNIFORMBUFFER_VIEWDATA_SIZE, BufferUsage::USAGE_DYNAMIC_DRAW, NULL);
		m_GlobalDataBuffer.Bind(UNIFORMBUFFER_VIEWDATA_BINDPOINT);

		// Construct the uniform buffer for lights.
		m_GlobalLightBuffer.Construct(m_RenderDevice, UNIFORMBUFFER_LIGHTDATA_SIZE, BufferUsage::USAGE_DYNAMIC_DRAW, NULL);
		m_GlobalLightBuffer.Bind(UNIFORMBUFFER_LIGHTDATA_BINDPOINT);

		// Construct the uniform buffer for debugging.
		m_GlobalDebugBuffer.Construct(m_RenderDevice, UNIFORMBUFFER_DEBUGDATA_SIZE, BufferUsage::USAGE_DYNAMIC_DRAW, NULL);
		m_GlobalDebugBuffer.Bind(UNIFORMBUFFER_DEBUGDATA_BINDPOINT);

		// Initialize the engine shaders.
		ConstructEngineShaders();

		// Initialize the render target.
		m_RenderTarget.Construct(m_RenderDevice);

		// Set default drawing parameters.
		m_DefaultDrawParams.primitiveType = PrimitiveType::PRIMITIVE_TRIANGLES;
		m_DefaultDrawParams.faceCulling = FaceCulling::FACE_CULL_BACK;
		m_DefaultDrawParams.shouldWriteDepth = true;
		m_DefaultDrawParams.depthFunc = DrawFunc::DRAW_FUNC_LESS;

		// Set skybox draw params.
		m_SkyboxDrawParams.primitiveType = PrimitiveType::PRIMITIVE_TRIANGLES;
		m_SkyboxDrawParams.faceCulling = FaceCulling::FACE_CULL_NONE;
		m_SkyboxDrawParams.shouldWriteDepth = true;
		m_SkyboxDrawParams.depthFunc = DrawFunc::DRAW_FUNC_LEQUAL;

		// Create a default texture for render context.
		Texture& text = CreateTexture("resources/textures/defaultDiffuse.png", PixelFormat::FORMAT_RGB, true, false, SamplerData());

		// Initialize the render context.
		m_DefaultRenderContext.Construct(m_RenderDevice, m_RenderTarget, m_DefaultDrawParams, text, m_LightingSystem);

		// Initialize skybox vertex array object.
		m_SkyboxVAO = m_RenderDevice.CreateSkyboxVertexArray();

		// Initialize ECS Camera System.
		Vector2F windowSize = Vector2F(m_MainWindow.GetWidth(), m_MainWindow.GetHeight());
		m_CameraSystem.Construct(ecsReg, m_DefaultRenderContext);
		m_CameraSystem.SetAspectRatio(windowSize.GetX() / windowSize.GetY());

		// Initialize ECS Mesh Renderer System
		m_MeshRendererSystem.Construct(ecsReg, m_DefaultRenderContext);

		// Initialize ECS Lighting system.
		m_LightingSystem.Construct(ecsReg, m_RenderDevice, *this);

		// Add the ECS systems into the pipeline.
		m_RenderingPipeline.AddSystem(m_CameraSystem);
		m_RenderingPipeline.AddSystem(m_MeshRendererSystem);
		m_RenderingPipeline.AddSystem(m_LightingSystem);

		// Set debug values.
		m_DebugData.visualizeDepth = true;

	}


	void RenderEngine::Tick(float delta)
	{
		// Clear color.
		m_DefaultRenderContext.Clear(m_CameraSystem.GetCurrentClearColor(), true);

		// Update pipeline.
		m_RenderingPipeline.UpdateSystems(delta);

		// Draw scene.
		m_DefaultRenderContext.Flush();

		// Update uniform buffers on GPU
		UpdateUniformBuffers();

		// Draw skybox.
		DrawSkybox();

		// Draw GUI Layers
		for (Layer* layer : m_GUILayerStack)
			layer->OnUpdate();

		// Update window.
		m_MainWindow.Tick();

	}

	void RenderEngine::OnWindowResized(float width, float height)
	{
		// Propogate to render device.
		m_RenderDevice.OnWindowResized(width, height);

		// Update camera system's projection matrix.
		Vector2F windowSize = Vector2F(m_MainWindow.GetWidth(), m_MainWindow.GetHeight());
		m_CameraSystem.SetAspectRatio(windowSize.GetX() / windowSize.GetY());
	}

	Material& RenderEngine::CreateMaterial(const std::string& materialName, Shaders shader)
	{
		if (!MaterialExists(materialName))
		{
			// Create material & set it's shader.
			return SetMaterialShader(m_LoadedMaterials[materialName], shader);
		}
		else
		{
			// Abort if material exists.
			LINA_CORE_ERR("Material with the name {0} already exists, returning that...", materialName);
			return m_LoadedMaterials[materialName];
		}
	}

	Texture& RenderEngine::CreateTexture(const std::string& filePath, PixelFormat pixelFormat, bool generateMipmaps, bool compress, SamplerData samplerData)
	{
		if (!TextureExists(filePath))
		{
			// Create pixel data.
			ArrayBitmap* textureBitmap = new ArrayBitmap();
			textureBitmap->Load(filePath);

			// Create texture & construct.
			m_LoadedTextures[filePath].Construct(m_RenderDevice, *textureBitmap, pixelFormat, generateMipmaps, compress, samplerData);

			// Delete pixel data.
			delete textureBitmap;

			// Return
			return m_LoadedTextures[filePath];
		}
		else
		{
			// Texture with this name already exists!
			LINA_CORE_ERR("Texture with the path {0} already exists, returning that...", filePath);
			return m_LoadedTextures[filePath];
		}
	}

	Texture& RenderEngine::CreateTexture(const std::string filePaths[6], PixelFormat pixelFormat, bool generateMipmaps, bool compress, SamplerData samplerData)
	{
		if (!TextureExists(filePaths[0]))
		{
			LinaArray<ArrayBitmap*> bitmaps;

			// Load bitmap for each face.
			for (uint32 i = 0; i < 6; i++)
			{
				ArrayBitmap* faceBitmap = new ArrayBitmap();
				faceBitmap->Load(filePaths[i]);
				bitmaps.push_back(faceBitmap);
			}

			// Create texture & construct.
			Texture& texture = m_LoadedTextures[filePaths[0]].Construct(m_RenderDevice, bitmaps, pixelFormat, generateMipmaps, compress, samplerData);

			// Delete pixel data.
			for (uint32 i = 0; i < bitmaps.size(); i++)
				delete bitmaps[i];

			// Clear list.
			bitmaps.clear();

			// Return
			return texture;
		}
		else
		{
			// Texture with this name already exists!
			LINA_CORE_ERR("Texture with the path {0} already exists, returning that...", filePaths[0]);
			return m_LoadedTextures[filePaths[0]];
		}
	}

	Mesh& RenderEngine::CreateMesh(const std::string& filePath)
	{
		if (!MeshExists(filePath))
		{

			// Create object data & feed it from model.
			Mesh& mesh = m_LoadedMeshes[filePath];
			m_ModelLoader.LoadModels(filePath, mesh.GetIndexedModels(), mesh.GetMaterialIndices(), mesh.GetMaterialSpecs());

			mesh.GetIndexedModels();
			if (mesh.GetIndexedModels().size() == 0)
				LINA_CORE_ERR("Indexed model array is empty! The model with the name: {0} could not be found or model scene does not contain any mesh! This will cause undefined behaviour or crashes if it is assigned to a ECS MeshRendererComponent."
					, filePath);

			// Create vertex array for each mesh.
			for (uint32 i = 0; i < mesh.GetIndexedModels().size(); i++)
			{
				VertexArray* vertexArray = new VertexArray();
				vertexArray->Construct(m_RenderDevice, mesh.GetIndexedModels()[i], BufferUsage::USAGE_STATIC_COPY);
				mesh.GetVertexArrays().push_back(vertexArray);
			}

			// Return
			return m_LoadedMeshes[filePath];
		}
		else
		{
			// Mesh with this name already exists!
			LINA_CORE_ERR("Mesh with the name {0} already exists, returning that...", filePath);
			return m_LoadedMeshes[filePath];
		}

	}

	Shader& RenderEngine::CreateShader(Shaders shader, const std::string& path)
	{
		// Create shader
		if (!ShaderExists(shader))
		{
			std::string shaderText;
			Utility::LoadTextFileWithIncludes(shaderText, path, "#include");
			return m_LoadedShaders[shader].Construct(m_RenderDevice, shaderText);
		}
		else
		{
			// Shader with this name already exists!
			LINA_CORE_WARN("Shader with the id {0} already exists, returning that...", shader);
			return m_LoadedShaders[shader];
		}
	}


	Material& RenderEngine::GetMaterial(const std::string& materialName)
	{
		if (!MaterialExists(materialName))
		{
			// Mesh not found.
			LINA_CORE_ERR("Material with the name {0} was not found, returning un-constructed material...", materialName);
			return Material();
		}

		return m_LoadedMaterials[materialName];
	}

	Texture& RenderEngine::GetTexture(const std::string& textureName)
	{
		if (!TextureExists(textureName))
		{
			// Mesh not found.
			LINA_CORE_ERR("Texture with the name {0} was not found, returning un-constructed texture...", textureName);
			return Texture();
		}

		return m_LoadedTextures[textureName];
	}

	Mesh& RenderEngine::GetMesh(const std::string& meshName)
	{
		if (!MeshExists(meshName))
		{
			// Mesh not found.
			LINA_CORE_ERR("Mesh with the name {0} was not found, returning un-constructed mesh...", meshName);
			return Mesh();
		}

		return m_LoadedMeshes[meshName];
	}

	Shader& RenderEngine::GetShader(Shaders shader)
	{
		if (!ShaderExists(shader))
		{
			// Shader not found.
			LINA_CORE_ERR("Shader with the name ID {0} was not found, returning standardLitShader", shader);
			return GetShader(Shaders::STANDARD_LIT);
		}

		return m_LoadedShaders[shader];
	}

	void RenderEngine::UnloadTextureResource(const std::string& textureName)
	{
		if (!TextureExists(textureName))
		{
			LINA_CORE_ERR("Texture not found! Aborting... ");
			return;
		}

		m_LoadedTextures.erase(textureName);
	}

	void RenderEngine::UnloadMeshResource(const std::string& meshName)
	{
		if (!MeshExists(meshName))
		{
			LINA_CORE_ERR("Mesh not found! Aborting... ");
			return;
		}

		m_LoadedMeshes.erase(meshName);
	}

	void RenderEngine::UnloadMaterialResource(const std::string& materialName)
	{
		if (!MaterialExists(materialName))
		{
			LINA_CORE_ERR("Material not found! Aborting... ");
			return;
		}

		m_LoadedMaterials.erase(materialName);
	}

	bool RenderEngine::MaterialExists(const std::string& materialName)
	{
		return !(m_LoadedMaterials.find(materialName) == m_LoadedMaterials.end());
	}

	bool RenderEngine::TextureExists(const std::string& textureName)
	{
		return !(m_LoadedTextures.find(textureName) == m_LoadedTextures.end());
	}

	bool RenderEngine::MeshExists(const std::string& meshName)
	{
		return !(m_LoadedMeshes.find(meshName) == m_LoadedMeshes.end());
	}

	bool RenderEngine::ShaderExists(Shaders shader)
	{
		return !(m_LoadedShaders.find(shader) == m_LoadedShaders.end());
	}


	void RenderEngine::ConstructEngineShaders()
	{
		// Unlit.
		Shader& unlit = CreateShader(Shaders::STANDARD_UNLIT, "resources/shaders/basicStandardUnlit.glsl");
		unlit.BindBlockToBuffer(UNIFORMBUFFER_VIEWDATA_BINDPOINT, UNIFORMBUFFER_VIEWDATA_NAME);
		unlit.BindBlockToBuffer(UNIFORMBUFFER_DEBUGDATA_BINDPOINT, UNIFORMBUFFER_DEBUGDATA_NAME);

		Shader& lit = CreateShader(Shaders::STANDARD_LIT, "resources/shaders/basicStandardLit.glsl");
		lit.BindBlockToBuffer(UNIFORMBUFFER_VIEWDATA_BINDPOINT, UNIFORMBUFFER_VIEWDATA_NAME);
		lit.BindBlockToBuffer(UNIFORMBUFFER_LIGHTDATA_BINDPOINT, UNIFORMBUFFER_LIGHTDATA_NAME);
		lit.BindBlockToBuffer(UNIFORMBUFFER_DEBUGDATA_BINDPOINT, UNIFORMBUFFER_DEBUGDATA_NAME);

		// Skies
		CreateShader(Shaders::SKYBOX_SINGLECOLOR, "resources/shaders/skyboxSingleColor.glsl");
		CreateShader(Shaders::SKYBOX_GRADIENT, "resources/shaders/skyboxVertexGradient.glsl").BindBlockToBuffer(UNIFORMBUFFER_VIEWDATA_BINDPOINT, UNIFORMBUFFER_VIEWDATA_NAME);
		CreateShader(Shaders::SKYBOX_CUBEMAP, "resources/shaders/skyboxCubemap.glsl").BindBlockToBuffer(UNIFORMBUFFER_VIEWDATA_BINDPOINT, UNIFORMBUFFER_VIEWDATA_NAME);
		CreateShader(Shaders::SKYBOX_PROCEDURAL, "resources/shaders/skyboxProcedural.glsl").BindBlockToBuffer(UNIFORMBUFFER_VIEWDATA_BINDPOINT, UNIFORMBUFFER_VIEWDATA_NAME);
	}

	void RenderEngine::DumpMemory()
	{
		// Clear dumps.
		m_LoadedMeshes.clear();
		m_LoadedTextures.clear();
		m_LoadedMaterials.clear();
	}

	void RenderEngine::DrawSkybox()
	{
		if (m_SkyboxMaterial != nullptr)
		{
			m_DefaultRenderContext.UpdateShaderData(m_SkyboxMaterial);
			m_DefaultRenderContext.Draw(m_SkyboxVAO, m_SkyboxDrawParams, 1, 36, true);
		}
	}


	void RenderEngine::UpdateUniformBuffers()
	{
		Vector3F cameraLocation = m_CameraSystem.GetCameraLocation();
		Vector4F viewPos = Vector4F(cameraLocation.GetX(), cameraLocation.GetY(), cameraLocation.GetZ(), 1.0f);

		// Update global matrix buffer
		m_GlobalDataBuffer.Update(&m_CameraSystem.GetProjectionMatrix(), 0, sizeof(Matrix));
		m_GlobalDataBuffer.Update(&m_CameraSystem.GetViewMatrix(), sizeof(Matrix), sizeof(Matrix));
		m_GlobalDataBuffer.Update(&viewPos, sizeof(Matrix) * 2, sizeof(Vector4F));

		// Update lights buffer.
		m_GlobalLightBuffer.Update(&m_CurrentPointLightCount, 0, sizeof(int));
		m_GlobalLightBuffer.Update(&m_CurrentSpotLightCount, sizeof(int), sizeof(int));

		// Update debug fufer.
		m_GlobalDebugBuffer.Update(&m_DebugData.visualizeDepth, 0, sizeof(bool));
	}

	Material& RenderEngine::SetMaterialShader(Material& material, Shaders shader)
	{

		// If no shader found, fall back to standardLit
		if (m_LoadedShaders.find(shader) == m_LoadedShaders.end()) {
			LINA_CORE_ERR("Shader with engine ID {0} was not found. Setting material's shader to standardLit.", shader);
			material.shaderID = m_LoadedShaders[Shaders::STANDARD_LIT].GetID();
		}
		else
			material.shaderID = m_LoadedShaders[shader].GetID();

		// Clear all shader related material data.
		material.textures.clear();
		material.colors.clear();
		material.floats.clear();
		material.ints.clear();
		material.vector3s.clear();
		material.vector2s.clear();
		material.matrices.clear();
		material.vector4s.clear();

		// Set shader data for material based on it's shader.
		if (shader == Shaders::STANDARD_LIT)
		{
			material.colors[MC_OBJECTCOLORPROPERTY] = Colors::White;
			material.floats[MC_SPECULARINTENSITYPROPERTY] = 2.0f;
			material.samplers[MC_DIFFUSETEXTUREPROPERTY] = 0;
			material.samplers[MC_SPECULARTEXTUREPROPERTY] = 1;
			material.ints[MC_SPECULAREXPONENTPROPERTY] = 32;
			material.receivesLighting = true;

		}
		else if (shader == Shaders::STANDARD_UNLIT)
		{
			material.colors[MC_OBJECTCOLORPROPERTY] = Colors::White;
			material.samplers[MC_DIFFUSETEXTUREPROPERTY] = 0;
		}
		else if (shader == Shaders::SKYBOX_SINGLECOLOR)
		{
			material.colors[MC_COLORPROPERTY] = Colors::White;
		}
		else if (shader == Shaders::SKYBOX_GRADIENT)
		{
			material.colors[MC_STARTCOLORPROPERTY] = Colors::Black;
			material.colors[MC_ENDCOLORPROPERTY] = Colors::White;
		}
		else if (shader == Shaders::SKYBOX_PROCEDURAL)
		{
			material.colors[MC_STARTCOLORPROPERTY] = Colors::Black;
			material.colors[MC_ENDCOLORPROPERTY] = Colors::White;
			material.vector3s[MC_SUNDIRECTIONPROPERTY] = Vector3F(0, -1, 0);
		}
		else if (shader == Shaders::SKYBOX_CUBEMAP)
		{
			material.samplers[MC_DIFFUSETEXTUREPROPERTY] = 0;
		}

		return material;
	}


	void RenderEngine::PushLayer(Layer* layer)
	{
		m_GUILayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void RenderEngine::PushOverlay(Layer* layer)
	{
		m_GUILayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

}