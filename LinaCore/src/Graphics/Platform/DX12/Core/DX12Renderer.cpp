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

#include "Graphics/Platform/DX12/Core/DX12Renderer.hpp"
#include "Graphics/Platform/DX12/SDK/D3D12MemAlloc.h"
#include "Graphics/Platform/DX12/Core/DX12Swapchain.hpp"
#include "Graphics/Platform/DX12/Core/DX12GfxBufferResource.hpp"
#include "Graphics/Platform/DX12/Core/DX12GfxTextureResource.hpp"
#include "Graphics/Core/GfxManager.hpp"
#include "Graphics/Resource/Shader.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Graphics/Resource/Material.hpp"
#include "FileSystem/FileSystem.hpp"
#include "System/ISystem.hpp"
#include "Resources/Core/ResourceManager.hpp"
#include "Math/Rect.hpp"
#include "Math/Color.hpp"
#include "Log/Log.hpp"

using Microsoft::WRL::ComPtr;

namespace Lina
{
	DWORD msgCallback = 0;

	void MessageCallback(D3D12_MESSAGE_CATEGORY messageType, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID messageId, LPCSTR pDesc, void* pContext)
	{
		if (pDesc != NULL)
		{
			if (severity == D3D12_MESSAGE_SEVERITY_ERROR)
			{
				LINA_ERR("[D3D12 Debug Layer] -> {0}", pDesc);
			}
			else if (severity == D3D12_MESSAGE_SEVERITY_WARNING)
			{
				LINA_WARN("[D3D12 Debug Layer] -> {0}", pDesc);
			}
			else if (severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
			{
				LINA_CRITICAL("[D3D12 Debug Layer] -> {0}", pDesc);
			}
			else if (severity == D3D12_MESSAGE_SEVERITY_MESSAGE)
			{
				LINA_INFO("[D3D12 Debug Layer] -> {0}", pDesc);
			}
		}
	}

	// ********************** BACKEND ********************************
	// ********************** BACKEND ********************************
	// ********************** BACKEND ********************************

	void Renderer::PreInitialize(const SystemInitializationInfo& initInfo, GfxManager* gfxMan)
	{
		m_gfxManager = gfxMan;

		{
			UINT dxgiFactoryFlags = 0;
			// Enable the debug layer (requires the Graphics Tools "optional feature").
			// NOTE: Enabling the debug layer after device creation will invalidate the active device.
#ifndef LINA_PRODUCTION_BUILD
			{
				ComPtr<ID3D12Debug> debugController;
				if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
				{
					debugController->EnableDebugLayer();
					// Enable additional debug layers.
					dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
				}
				else
				{
					LINA_ERR("[Gfx Backend] -> Failed enabling debug layers!");
				}
			}
#endif
			ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
		}

		// Choose gpu & create device
		{
			if (initInfo.preferredGPUType == PreferredGPUType::CPU)
			{
				ComPtr<IDXGIAdapter> warpAdapter;
				ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

				ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
			}
			else
			{
				GetHardwareAdapter(m_factory.Get(), &m_adapter, initInfo.preferredGPUType);

				ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
			}
		}

#ifndef LINA_PRODUCTION_BUILD
		// Dbg callback
		{
			ID3D12InfoQueue1* infoQueue = nullptr;
			if (SUCCEEDED(m_device->QueryInterface<ID3D12InfoQueue1>(&infoQueue)))
			{
				infoQueue->RegisterMessageCallback(&MessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &msgCallback);
			}
		}
#endif

		// Describe and create the command queues.
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags					   = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type					   = D3D12_COMMAND_LIST_TYPE_DIRECT;
			ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_graphicsQueue)));

			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_copyQueue)));
		}

		// Heaps
		{
			m_bufferHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 100);
			m_dsvHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 10);
			m_rtvHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 10);
			m_samplerHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 100);
		}

		// Allocator
		{
			D3D12MA::ALLOCATOR_DESC allocatorDesc;
			allocatorDesc.pDevice			   = m_device.Get();
			allocatorDesc.PreferredBlockSize   = 0;
			allocatorDesc.Flags				   = D3D12MA::ALLOCATOR_FLAG_NONE;
			allocatorDesc.pAdapter			   = m_adapter.Get();
			allocatorDesc.pAllocationCallbacks = NULL;
			ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_dx12Allocator));
		}
	}

	void Renderer::Initialize(const SystemInitializationInfo& initInfo)
	{
		// DXC
		{
			HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_idxcLib));
			if (FAILED(hr))
			{
				LINA_CRITICAL("[D3D12 Gpu Storage] -> Failed to create DXC library!");
			}
		}

		// Standard Pipeline Root Signature
		{
			/*******************************************************/
			/** PARAM0: CBV				-> Global Data			-> REG:b0 -> per frame
			/**
			/** PARAM1: CBV				-> Scene Data			-> REG:b1 -> per scene
			/**
			/** PARAM2: CBV				-> View Data			-> REG:b2 -> per render target
			/**
			/** PARAM3: CBV				-> Material Data        -> REG:b3 -> per pipeline
			/**
			/** PARAM4: Sampler Table	-> Material Samplers	-> REG:[s0-s9] -> per pipeline
			/**
			/** PARAM5: SRV Table		-> Material Textures	-> REG:[t0-t9] -> per pipeline
			/**
			/** PARAM6: SRV				-> Object Buffer View	-> REG:t10 -> per scene
			/*******************************************************/

			CD3DX12_DESCRIPTOR_RANGE1 descRange[2] = {};

			// Textures & Samplers
			descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
			descRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 10, 0);

			CD3DX12_ROOT_PARAMETER1 rp[7] = {};

			// Global constant buffer
			rp[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

			// Scene data
			rp[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

			// View data
			rp[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

			// Material data
			rp[3].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_ALL);

			// Texture & samplers
			rp[4].InitAsDescriptorTable(1, &descRange[0], D3D12_SHADER_VISIBILITY_ALL);
			rp[5].InitAsDescriptorTable(1, &descRange[1], D3D12_SHADER_VISIBILITY_ALL);

			// Object buffer.
			rp[6].InitAsShaderResourceView(10, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
			samplerDesc[0].Init(10, D3D12_FILTER_ANISOTROPIC);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSig(7, rp, 1, samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
			ComPtr<ID3DBlob>					  signatureBlob = nullptr;
			ComPtr<ID3DBlob>					  errorBlob		= nullptr;

			HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSig, &signatureBlob, &errorBlob);

			if (FAILED(hr))
			{
				LINA_CRITICAL("[Shader Compiler] -> Failed serializing root signature! {0}", (char*)errorBlob->GetBufferPointer());
			}

			hr = m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSigStandard));

			if (FAILED(hr))
			{
				LINA_CRITICAL("[Shader Compiler] -> Failed creating root signature!");
			}
		}

		// Indirect command signature
		{
			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
			argumentDescs[0].Type						  = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs				  = argumentDescs;
			commandSignatureDesc.NumArgumentDescs			  = _countof(argumentDescs);
			commandSignatureDesc.ByteStride					  = sizeof(DrawIndexedIndirectCommand);

			ThrowIfFailed(m_device->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&m_commandSigStandard)));
		}

		// Sycnronization resources
		{
			m_frameFenceGraphics = CreateFence();
			m_fenceValueGraphics = 1;

			m_fenceEventGraphics = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEventGraphics == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}
		}

		m_uploader.Initialize();
	}

	void Renderer::Shutdown()
	{
		m_uploader.Shutdown();
		ReleaseFence(m_frameFenceGraphics);

		ID3D12InfoQueue1* infoQueue = nullptr;
		if (SUCCEEDED(m_device->QueryInterface<ID3D12InfoQueue1>(&infoQueue)))
		{
			infoQueue->UnregisterMessageCallback(msgCallback);
		}

		m_graphicsQueue.Reset();
		m_copyQueue.Reset();
		m_dx12Allocator->Release();
		m_device.Reset();
	}

	void Renderer::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, PreferredGPUType gpuType)
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, gpuType == PreferredGPUType::Discrete ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					char* buf = FileSystem::WCharToChar(desc.Description);
					LINA_TRACE("[D3D12 Backend] -> Selected hardware adapter: {0}, Dedicated Video Memory: {1}", buf, desc.DedicatedVideoMemory * 0.000001);
					delete[] buf;
					break;
				}
			}
		}

		if (adapter.Get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					char* buf = FileSystem::WCharToChar(desc.Description);
					LINA_TRACE("[D3D12 Backend] -> Selected hardware adapter: {0}, Dedicated Video Memory: {1} MB", buf, desc.DedicatedVideoMemory * 0.000001);
					delete[] buf;
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}

	// ********************** RESOURCES ********************************
	// ********************** RESOURCES ********************************
	// ********************** RESOURCES ********************************

	LPCWSTR macros[6] = {{L"LINA_PASS_OPAQUE="}, {L"LINA_PASS_SHADOWS="}, {L"LINA_PIPELINE_STANDARD="}, {L"LINA_PIPELINE_NOVERTEX="}, {L"LINA_PIPELINE_GUI="}, {L"LINA_MATERIAL=cbuffer MaterialBuffer : register(b3)"}};

	uint32 Renderer::GenerateMaterial(Material* mat, uint32 existingHandle)
	{
		LOCK_GUARD(m_shaderMtx);

		const uint32	   index   = existingHandle == -1 ? m_materials.AddItem(GeneratedMaterial()) : existingHandle;
		GeneratedMaterial& genData = m_materials.GetItemR(index);

		return index;
	}

	void Renderer::UpdateMaterialProperties(Material* mat, uint32 imageIndex)
	{
	}

	void Renderer::UpdateMaterialTextures(Material* mat, uint32 imageIndex, const Vector<uint32>& dirtyTextures)
	{
	}

	void Renderer::DestroyMaterial(uint32 handle)
	{
		// Note: no need to mtx lock, this is called from the main thread.
		const uint32 index	 = handle;
		auto&		 genData = m_materials.GetItemR(index);
		m_materials.RemoveItem(index);
	}

	uint32 Renderer::GeneratePipeline(Shader* shader)
	{
		LOCK_GUARD(m_shaderMtx);

		const uint32	   index   = m_shaders.AddItem(GeneratedShader());
		auto&			   genData = m_shaders.GetItemR(index);
		const auto&		   stages  = shader->GetStages();
		const PipelineType ppType  = shader->GetPipelineType();

		Vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

		// Define the vertex input layout.
		{
			if (ppType == PipelineType::Standard)
			{
				inputLayout.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
				inputLayout.push_back({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
				inputLayout.push_back({"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
				inputLayout.push_back({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
			}
			else if (ppType == PipelineType::GUI)
			{
				inputLayout.push_back({"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
				inputLayout.push_back({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
				inputLayout.push_back({"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
			}
			else if (ppType == PipelineType::NoVertex)
			{
				// no input.
			}
		}

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		if (!inputLayout.empty())
			psoDesc.InputLayout = {&inputLayout[0], static_cast<UINT>(inputLayout.size())};

		psoDesc.pRootSignature					= m_rootSigStandard.Get();
		psoDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState						= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable	= FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask						= UINT_MAX;
		psoDesc.PrimitiveTopologyType			= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets				= 1;
		psoDesc.RTVFormats[0]					= GetFormat(DEFAULT_SWAPCHAIN_FORMAT);
		psoDesc.SampleDesc.Count				= 1;
		psoDesc.RasterizerState.FillMode		= D3D12_FILL_MODE_SOLID;

		if (ppType == PipelineType::Standard)
		{
			psoDesc.RasterizerState.CullMode			  = D3D12_CULL_MODE_BACK;
			psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		}
		else if (ppType == PipelineType::NoVertex)
		{
			psoDesc.RasterizerState.CullMode			  = D3D12_CULL_MODE_NONE;
			psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		}
		else if (ppType == PipelineType::GUI)
		{
			psoDesc.RasterizerState.CullMode			  = D3D12_CULL_MODE_NONE;
			psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		}

		for (const auto& [stg, title] : stages)
		{
			const auto&	 bc		  = shader->GetCompiledCode(stg);
			const void*	 byteCode = (void*)bc.data;
			const SIZE_T length	  = static_cast<SIZE_T>(bc.dataSize);

			if (stg == ShaderStage::Vertex)
			{
				psoDesc.VS.pShaderBytecode = byteCode;
				psoDesc.VS.BytecodeLength  = length;
			}
			else if (stg == ShaderStage::Fragment)
			{
				psoDesc.PS.pShaderBytecode = byteCode;
				psoDesc.PS.BytecodeLength  = length;
			}
			else if (stg == ShaderStage::Geometry)
			{
				psoDesc.GS.pShaderBytecode = byteCode;
				psoDesc.GS.BytecodeLength  = length;
			}
			else if (stg == ShaderStage::TesellationControl || stg == ShaderStage::TesellationEval)
			{
				psoDesc.HS.pShaderBytecode = byteCode;
				psoDesc.HS.BytecodeLength  = length;
			}
		}

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&genData.pso)));

		return index;
	}

	void Renderer::DestroyPipeline(uint32 handle)
	{
		const uint32 index	 = handle;
		auto&		 genData = m_shaders.GetItemR(index);

		m_shaders.RemoveItem(index);
	}

	void Renderer::CompileShader(const char* path, const HashMap<ShaderStage, String>& stages, HashMap<ShaderStage, ShaderByteCode>& outCompiledCode)
	{

		Microsoft::WRL::ComPtr<IDxcCompiler3> idxcCompiler;

		HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&idxcCompiler));
		if (FAILED(hr))
		{
			LINA_CRITICAL("[Shader Compile] -> Failed to create DXC compiler");
		}

		ComPtr<IDxcUtils> utils;
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()));

		if (FAILED(hr))
		{
			LINA_CRITICAL("[Shader Compile] -> Failed to create DXC utils");
		}

		wchar_t* pathw = FileSystem::CharToWChar(path);

		// IDxcCompilerArgs* pCompilerArgs;
		// DxcCreateInstance(CLSID_DxcCompilerArgs, IID_PPV_ARGS(&pCompilerArgs));
		// const wchar_t* arg = L"/O3";
		// pCompilerArgs->AddArguments(&arg, 1);

		for (auto& [stage, title] : stages)
		{
			UINT32					 codePage = CP_UTF8;
			ComPtr<IDxcBlobEncoding> sourceBlob;
			ThrowIfFailed(m_idxcLib->CreateBlobFromFile(pathw, &codePage, &sourceBlob));

			wchar_t* entry = FileSystem::CharToWChar(title.c_str());

			const wchar_t* targetProfile = L"vs_6_0";
			if (stage == ShaderStage::Fragment)
				targetProfile = L"ps_6_0";
			else if (stage == ShaderStage::Compute)
				targetProfile = L"cs_6_0";
			else if (stage == ShaderStage::Geometry)
				targetProfile = L"gs_6_0";
			else if (stage == ShaderStage::TesellationControl || stage == ShaderStage::TesellationEval)
				targetProfile = L"hs_6_0";

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr	  = sourceBlob->GetBufferPointer();
			sourceBuffer.Size	  = sourceBlob->GetBufferSize();
			sourceBuffer.Encoding = 0;

			Vector<LPCWSTR> arguments;

			arguments.push_back(L"-E");
			arguments.push_back(entry);
			arguments.push_back(L"-T");
			arguments.push_back(targetProfile);
			arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX

			// We will still get reflection info, just with a smaller binary.
			arguments.push_back(L"-Qstrip_debug");
			arguments.push_back(L"-Qstrip_reflect");

#ifndef LINA_PRODUCTION_BUILD
			arguments.push_back(DXC_ARG_DEBUG); //-Zi
#else
			arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

			for (int i = 0; i < 6; i++)
			{
				arguments.push_back(L"-D");
				arguments.push_back(macros[i]);
			}

			ComPtr<IDxcResult> result;
			ThrowIfFailed(idxcCompiler->Compile(&sourceBuffer, arguments.data(), static_cast<uint32>(arguments.size()), &m_includeInterface, IID_PPV_ARGS(result.GetAddressOf())));

			ComPtr<IDxcBlobUtf8> errors;
			ComPtr<IDxcBlobWide> outputName;
			result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);

			if (errors && errors->GetStringLength() > 0)
			{
				LINA_ERR("[Shader Compile Error] -> {0} {1}", path, (char*)errors->GetStringPointer());
			}

			// ComPtr<IDxcBlob> reflectionData;
			// result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflectionData.GetAddressOf()), nullptr);
			// DxcBuffer reflectionBuffer;
			// reflectionBuffer.Ptr	  = reflectionData->GetBufferPointer();
			// reflectionBuffer.Size	  = reflectionData->GetBufferSize();
			// reflectionBuffer.Encoding = 0;
			// ComPtr<ID3D12ShaderReflection> shaderReflection;
			// utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(shaderReflection.GetAddressOf()));

			result->GetStatus(&hr);

			if (FAILED(hr))
			{
				if (result)
				{
					ComPtr<IDxcBlobEncoding> errorsBlob;
					hr = result->GetErrorBuffer(&errorsBlob);
					if (SUCCEEDED(hr) && errorsBlob)
					{
						LINA_ERR("[D3D12GpUStorage]-> Shader compilation failed: {0} {1}", path, (const char*)errorsBlob->GetBufferPointer());
					}
				}
			}

			ComPtr<IDxcBlob> code;
			result->GetResult(&code);

			const SIZE_T sz = code->GetBufferSize();
			auto&		 bc = outCompiledCode[stage];
			bc.dataSize		= static_cast<uint32>(sz);
			bc.data			= new uint8[sz];
			MEMCPY(bc.data, code->GetBufferPointer(), sz);

			delete[] entry;
		}

		delete[] pathw;
	}

	uint32 Renderer::GenerateImage(Texture* txt, ImageType type)
	{
		LOCK_GUARD(m_textureMtx);

		const uint32	  index	  = m_textures.AddItem(GeneratedTexture());
		GeneratedTexture& genData = m_textures.GetItemR(index);
		auto&			  meta	  = txt->GetMetadata();
		const auto&		  ext	  = txt->GetExtent();
		genData.imageType		  = type;

		// Rest will be done by CreateRenderTarget
		if (type == ImageType::Swapchain)
		{
			genData.descriptor = m_rtvHeap.Allocate();
		}
		else if (type == ImageType::DepthStencil)
		{
			genData.descriptor	= m_dsvHeap.Allocate();
			genData.gfxResource = CreateTextureResource(TextureResourceType::Texture2DDepthStencil, Vector2i(ext.width, ext.height));
		}
		else
		{
		}

		return index;
	}

	uint32 Renderer::GenerateImageAndUpload(Texture* txt)
	{
		LOCK_GUARD(m_textureMtx);

		const uint32	  index	  = m_textures.AddItem(GeneratedTexture());
		GeneratedTexture& genData = m_textures.GetItemR(index);
		auto&			  meta	  = txt->GetMetadata();
		const auto&		  ext	  = txt->GetExtent();

		return index;
	}

	void Renderer::DestroyImage(uint32 handle)
	{
		const uint32	  index	  = handle;
		GeneratedTexture& genData = m_textures.GetItemR(index);

		if (genData.imageType == ImageType::Swapchain)
		{
			m_rtvHeap.Free(genData.descriptor);
			genData.rawResource.Reset();

			if (genData.gfxResource)
				delete genData.gfxResource;
		}

		m_textures.RemoveItem(index);
	}

	void Renderer::BeginFrame(uint32 frameIndex)
	{
		// Will wait if pending commands.
		m_uploader.Flush();

		Renderer::WaitForFences(m_frameFenceGraphics, m_fenceValueGraphics, m_frames[frameIndex].storedFenceGraphics);
	}

	void Renderer::EndFrame(uint32 frameIndex)
	{
		auto& fence				  = m_fences.GetItemR(m_frameFenceGraphics);
		auto& frame				  = m_frames[frameIndex];
		frame.storedFenceGraphics = m_fenceValueGraphics;
		m_graphicsQueue->Signal(fence.Get(), m_fenceValueGraphics);
		m_fenceValueGraphics++;
	}

	void Renderer::WaitForGPUGraphics()
	{
		auto& fence = m_fences.GetItemR(m_frameFenceGraphics);

		// Schedule a Signal command in the queue.
		ThrowIfFailed(m_graphicsQueue->Signal(fence.Get(), m_fenceValueGraphics));

		// Wait until the fence has been processed.
		ThrowIfFailed(fence->SetEventOnCompletion(m_fenceValueGraphics, m_fenceEventGraphics));
		WaitForSingleObjectEx(m_fenceEventGraphics, INFINITE, FALSE);
	}

	ISwapchain* Renderer::CreateSwapchain(const Vector2i& size, void* windowHandle)
	{
		return new DX12Swapchain(this, size, windowHandle);
	}

	uint32 Renderer::CreateCommandAllocator(CommandType type)
	{
		const uint32 handle = m_cmdAllocators.AddItem(ComPtr<ID3D12CommandAllocator>());
		auto&		 alloc	= m_cmdAllocators.GetItemR(handle);
		ThrowIfFailed(m_device->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(alloc.GetAddressOf())));
		return handle;
	}

	uint32 Renderer::CreateCommandList(CommandType type, uint32 allocatorHandle)
	{
		const uint32 handle	   = m_cmdLists.AddItem(ComPtr<ID3D12GraphicsCommandList4>());
		auto&		 list	   = m_cmdLists.GetItemR(handle);
		auto&		 allocator = m_cmdAllocators.GetItemR(allocatorHandle);
		ThrowIfFailed(m_device->CreateCommandList(0, GetCommandType(type), allocator.Get(), nullptr, IID_PPV_ARGS(list.GetAddressOf())));
		ThrowIfFailed(list->Close());
		return handle;
	}

	uint32 Renderer::CreateFence()
	{
		const uint32 handle = m_fences.AddItem(ComPtr<ID3D12Fence>());
		auto&		 fence	= m_fences.GetItemR(handle);
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		return handle;
	}

	void Renderer::ReleaseCommanAllocator(uint32 handle)
	{
		m_cmdAllocators.GetItemR(handle).Reset();
		m_cmdAllocators.RemoveItem(handle);
	}

	IGfxBufferResource* Renderer::CreateBufferResource(BufferResourceType type, void* initialData, size_t size)
	{
		return new DX12GfxBufferResource(this, type, initialData, size);
	}

	IGfxTextureResource* Renderer::CreateTextureResource(TextureResourceType type, const Vector2i& initialSize)
	{
		return new DX12GfxTextureResource(this, type, initialSize);
	}

	void Renderer::DeleteBufferResource(IGfxBufferResource* res)
	{
		delete res;
	}

	void Renderer::ReleaseCommandList(uint32 handle)
	{
		m_cmdLists.GetItemR(handle).Reset();
		m_cmdLists.RemoveItem(handle);
	}

	void Renderer::ResetCommandList(uint32 cmdAllocatorHandle, uint32 cmdListHandle)
	{
		auto& cmdList	   = m_cmdLists.GetItemR(cmdListHandle);
		auto& cmdAllocator = m_cmdAllocators.GetItemR(cmdAllocatorHandle);
		ThrowIfFailed(cmdAllocator->Reset());
		ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));
	}

	void Renderer::PrepareCommandList(uint32 cmdListHandle, const Viewport& viewport, const Recti& scissors)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		// ID3D12DescriptorHeap* heaps[] = {bufferHeap.GetHeap()};
		cmdList->SetGraphicsRootSignature(m_rootSigStandard.Get());
		//	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

		// Viewport & scissors.
		{
			D3D12_VIEWPORT vp;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.Height	= viewport.height;
			vp.Width	= viewport.width;
			vp.TopLeftX = viewport.x;
			vp.TopLeftY = viewport.y;

			D3D12_RECT sc;
			sc.left	  = static_cast<LONG>(scissors.pos.x);
			sc.top	  = static_cast<LONG>(scissors.pos.y);
			sc.right  = static_cast<LONG>(scissors.size.x);
			sc.bottom = static_cast<LONG>(scissors.size.y);

			cmdList->RSSetViewports(1, &vp);
			cmdList->RSSetScissorRects(1, &sc);
		}
	}

	void Renderer::FinalizeCommandList(uint32 cmdListHandle)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		ThrowIfFailed(cmdList->Close());
	}

	void Renderer::ExecuteCommandListsGraphics(const Vector<uint32>& lists)
	{
		const UINT sz = static_cast<UINT>(lists.size());

		Vector<ID3D12CommandList*> _lists;

		for (UINT i = 0; i < sz; i++)
		{
			auto& lst = m_cmdLists.GetItemR(lists[i]);
			_lists.push_back(lst.Get());
		}

		ID3D12CommandList* const* data = _lists.data();
		m_graphicsQueue->ExecuteCommandLists(sz, data);
	}

	void Renderer::ExecuteCommandListsTransfer(const Vector<uint32>& lists)
	{
		const UINT sz = static_cast<UINT>(lists.size());

		Vector<ID3D12CommandList*> _lists;

		for (UINT i = 0; i < sz; i++)
		{
			auto& lst = m_cmdLists.GetItemR(lists[i]);
			_lists.push_back(lst.Get());
		}

		ID3D12CommandList* const* data = _lists.data();
		m_copyQueue->ExecuteCommandLists(sz, data);
	}

	void Renderer::TransitionPresent2RT(uint32 cmdListHandle, Texture* txt)
	{
		auto& cmdList	  = m_cmdLists.GetItemR(cmdListHandle);
		auto& genData	  = m_textures.GetItemR(txt->GetGPUHandle());
		auto  presentToRT = CD3DX12_RESOURCE_BARRIER::Transition(genData.rawResource.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->ResourceBarrier(1, &presentToRT);
	}

	void Renderer::TransitionRT2Present(uint32 cmdListHandle, Texture* txt)
	{
		auto& cmdList	  = m_cmdLists.GetItemR(cmdListHandle);
		auto& genData	  = m_textures.GetItemR(txt->GetGPUHandle());
		auto  rtToPresent = CD3DX12_RESOURCE_BARRIER::Transition(genData.rawResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		cmdList->ResourceBarrier(1, &rtToPresent);
	}

	void Renderer::BeginRenderPass(uint32 cmdListHandle, Texture* colorTexture, const Color& clearColor)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		auto& genData = m_textures.GetItemR(colorTexture->GetGPUHandle());

		const float			cc[]{clearColor.x, clearColor.y, clearColor.z, clearColor.w};
		CD3DX12_CLEAR_VALUE clearValue{GetFormat(DEFAULT_COLOR_FORMAT), cc};

		D3D12_RENDER_PASS_BEGINNING_ACCESS	 renderPassBeginningAccessClear{D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, {clearValue}};
		D3D12_RENDER_PASS_ENDING_ACCESS		 renderPassEndingAccessPreserve{D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {}};
		D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc{genData.descriptor.cpuHandle, renderPassBeginningAccessClear, renderPassEndingAccessPreserve};

		cmdList->BeginRenderPass(1, &renderPassRenderTargetDesc, nullptr, D3D12_RENDER_PASS_FLAG_NONE);
	}

	void Renderer::BeginRenderPass(uint32 cmdListHandle, Texture* colorTexture, Texture* depthStencilTexture, const Color& clearColor)
	{
		auto& cmdList			  = m_cmdLists.GetItemR(cmdListHandle);
		auto& genDataCol		  = m_textures.GetItemR(colorTexture->GetGPUHandle());
		auto& genDataDepthStencil = m_textures.GetItemR(depthStencilTexture->GetGPUHandle());

		const float			cc[]{clearColor.x, clearColor.y, clearColor.z, clearColor.w};
		CD3DX12_CLEAR_VALUE clearValue{GetFormat(DEFAULT_COLOR_FORMAT), cc};

		D3D12_RENDER_PASS_BEGINNING_ACCESS	 renderPassBeginningAccessClear{D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, {clearValue}};
		D3D12_RENDER_PASS_ENDING_ACCESS		 renderPassEndingAccessPreserve{D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {}};
		D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc{genDataCol.descriptor.cpuHandle, renderPassBeginningAccessClear, renderPassEndingAccessPreserve};

		D3D12_RENDER_PASS_BEGINNING_ACCESS	 renderPassBeginningAccessNoAccess{D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {}};
		D3D12_RENDER_PASS_ENDING_ACCESS		 renderPassEndingAccessNoAccess{D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {}};
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc{genDataDepthStencil.descriptor.cpuHandle, renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess, renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess};

		cmdList->BeginRenderPass(1, &renderPassRenderTargetDesc, nullptr, D3D12_RENDER_PASS_FLAG_NONE);
	}

	void Renderer::EndRenderPass(uint32 cmdListHandle)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->EndRenderPass();
	}

	void Renderer::BindUniformBuffer(uint32 cmdListHandle, uint32 bufferIndex, IGfxBufferResource* buf)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->SetGraphicsRootConstantBufferView(bufferIndex, buf->GetGPUPointer());
	}

	void Renderer::BindObjectBuffer(uint32 cmdListHandle, IGfxBufferResource* buf)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->SetGraphicsRootShaderResourceView(6, buf->GetGPUPointer());
	}

	void Renderer::BindMaterial(uint32 cmdListHandle, Material* mat)
	{
		auto* shader = mat->GetShader();

		if (shader == nullptr)
		{
			LINA_ERR("[Renderer] -> Can not bind material because the material does not have an assigned shader!");
			return;
		}

		auto& cmdList	 = m_cmdLists.GetItemR(cmdListHandle);
		auto& shaderData = m_shaders.GetItemR(shader->GetGPUHandle());
		cmdList->SetPipelineState(shaderData.pso.Get());
	}

	void Renderer::DrawInstanced(uint32 cmdListHandle, uint32 vertexCount, uint32 instanceCount, uint32 startVertex, uint32 startInstance)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
	}

	void Renderer::DrawIndexedInstanced(uint32 cmdListHandle, uint32 indexCountPerInstance, uint32 instanceCount, uint32 startIndexLocation, uint32 baseVertexLocation, uint32 startInstanceLocation)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void Renderer::DrawIndexedIndirect(uint32 cmdListHandle, IGfxBufferResource* indirectBuffer, uint32 count, uint64 indirectOffset)
	{
		auto&				   cmdList = m_cmdLists.GetItemR(cmdListHandle);
		DX12GfxBufferResource* buf	   = static_cast<DX12GfxBufferResource*>(indirectBuffer);

		D3D12_RESOURCE_BARRIER barriers[1] = {CD3DX12_RESOURCE_BARRIER::Transition(buf->DX12GetAllocation()->GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)};
		cmdList->ResourceBarrier(_countof(barriers), barriers);
		cmdList->ExecuteIndirect(m_commandSigStandard.Get(), count, buf->DX12GetAllocation()->GetResource(), indirectOffset, nullptr, 0);
	}

	void Renderer::SetTopology(uint32 cmdListHandle, Topology topology)
	{
		auto& cmdList = m_cmdLists.GetItemR(cmdListHandle);
		cmdList->IASetPrimitiveTopology(GetTopology(topology));
	}

	void Renderer::PushTransferCommand(GfxCommand& cmd)
	{
		m_uploader.PushCommand(cmd);
	}

	void Renderer::BindVertexBuffer(uint32 cmdListHandle, IGfxBufferResource* buffer, uint32 slot)
	{
		auto&					 cmdList = m_cmdLists.GetItemR(cmdListHandle);
		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = buffer->GetGPUPointer();
		view.SizeInBytes	= buffer->GetSize();
		view.StrideInBytes	= sizeof(Vertex);
		cmdList->IASetVertexBuffers(slot, 1, &view);
	}

	void Renderer::BindIndexBuffer(uint32 cmdListHandle, IGfxBufferResource* buffer)
	{
		auto&					cmdList = m_cmdLists.GetItemR(cmdListHandle);
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = buffer->GetGPUPointer();
		view.SizeInBytes	= static_cast<UINT>(buffer->GetSize());
		view.Format			= DXGI_FORMAT_R32_UINT;
		cmdList->IASetIndexBuffer(&view);
	}

	void Renderer::CopyCPU2GPU(IGfxBufferResource* cpuBuffer, IGfxBufferResource* gpuBuffer, void* data, size_t sz, ResourceState finalState)
	{
		GfxCommand cmd;

		DX12GfxBufferResource* cpuRes = static_cast<DX12GfxBufferResource*>(cpuBuffer);
		DX12GfxBufferResource* gpuRes = static_cast<DX12GfxBufferResource*>(gpuBuffer);

		cmd.Record = [data, sz, cpuRes, gpuRes, finalState, this](uint32 list) {
			auto& cmdList = m_cmdLists.GetItemR(list);

			D3D12_SUBRESOURCE_DATA sData = {};
			sData.pData					 = data;
			sData.RowPitch				 = sz;
			sData.SlicePitch			 = sData.RowPitch;

			UpdateSubresources<1>(cmdList.Get(), gpuRes->DX12GetAllocation()->GetResource(), cpuRes->DX12GetAllocation()->GetResource(), 0, 0, 1, &sData);
			auto fs			= GetResourceState(finalState);
			auto transition = CD3DX12_RESOURCE_BARRIER::Transition(gpuRes->DX12GetAllocation()->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, fs);
			cmdList->ResourceBarrier(1, &transition);
		};

		PushTransferCommand(cmd);
	}

	void Renderer::WaitForCopyQueue()
	{
		m_uploader.Flush();
	}

	void Renderer::Present(ISwapchain* swp)
	{
		DX12Swapchain* dx12Swap = static_cast<DX12Swapchain*>(swp);

		try
		{
			dx12Swap->GetPtr()->Present(1, 0);
		}
		catch (HrException& e)
		{
			if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
			{
			}
			else
			{
				throw;
			}
		}
	}

	uint32 Renderer::GetNextBackBuffer(ISwapchain* swp)
	{
		DX12Swapchain* dx12Swap = static_cast<DX12Swapchain*>(swp);
		return static_cast<uint32>(dx12Swap->GetPtr()->GetCurrentBackBufferIndex());
	}

	void Renderer::ReleaseFence(uint32 handle)
	{
		m_fences.GetItemR(handle).Reset();
		m_fences.RemoveItem(handle);
	}

	void Renderer::WaitForFences(uint32 fenceHandle, uint64 userData0, uint64 userData1)
	{
		// UserData0: Current Fence Value in caller
		// UserData1: Stored fence value in caller's frame

		auto&		 fence	   = m_fences.GetItemR(fenceHandle);
		const UINT64 lastFence = fence->GetCompletedValue();

		if (userData1 > lastFence)
		{
			HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (eventHandle == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}
			else
			{
				ThrowIfFailed(fence->SetEventOnCompletion(userData1, eventHandle));
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
		}
	}

	Texture* Renderer::CreateRenderTarget(const String& path)
	{
		const StringID sid = TO_SID(path);
		Texture*	   rt  = new Texture(m_gfxManager->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager), true, path, sid);
		// TODO
		return rt;
	}

	Texture* Renderer::CreateRenderTarget(ISwapchain* swp, uint32 bufferIndex, const String& path)
	{
		const StringID sid = TO_SID(path);
		Texture*	   rt  = new Texture(m_gfxManager->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager), true, path, sid);
		rt->GenerateImage(ImageType::Swapchain);
		auto& genData = m_textures.GetItemR(rt->GetGPUHandle());

		DX12Swapchain* dx12Swap = static_cast<DX12Swapchain*>(swp);
		ThrowIfFailed(dx12Swap->GetPtr()->GetBuffer(bufferIndex, IID_PPV_ARGS(&genData.rawResource)));
		m_device->CreateRenderTargetView(genData.rawResource.Get(), nullptr, genData.descriptor.cpuHandle);

		return rt;
	}

	Texture* Renderer::CreateDepthStencil(const String& path)
	{
		const StringID sid = TO_SID(path);
		Texture*	   rt  = new Texture(m_gfxManager->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager), true, path, sid);
		rt->GenerateImage(ImageType::DepthStencil);
		auto& genData = m_textures.GetItemR(rt->GetGPUHandle());

		DX12GfxTextureResource* res = static_cast<DX12GfxTextureResource*>(genData.gfxResource);

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format						   = GetFormat(DEFAULT_DEPTH_FORMAT);
		depthStencilDesc.ViewDimension				   = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags						   = D3D12_DSV_FLAG_NONE;
		m_device->CreateDepthStencilView(res->DX12GetAllocation()->GetResource(), &depthStencilDesc, genData.descriptor.cpuHandle);

		return rt;
	}

	// void Renderer::BindPipeline(ID3D12GraphicsCommandList* cmdList, Material* mat)
	// {
	// 	GeneratedShader& genData = shaders.GetItemR(mat->GetShader()->GetGPUHandle());
	// 	cmdList->SetPipelineState(genData.pso.Get());
	// }

} // namespace Lina
