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

#include "Common/Platform/LinaGXIncl.hpp"
#include "Core/Graphics/Pipeline/RenderPass.hpp"

namespace Lina::Editor
{
	struct GPUEditorGUIPushConstants
	{
		uint32 materialIndex;
	};

	struct GPUMaterialGUIColorWheel
	{
		float wheelRadius;
		float edgeSmoothness;
	};

	struct GPUMaterialGUIDefault
	{
		Vector4				 uvTilingAndOffset;
		uint32				 hasTexture;
		uint32				 displayChannels;
		uint32				 displayLod;
		uint32				 singleChannel;
		LinaTexture2DBinding diffuse;
	};

	struct GPUMaterialGUIGlitch
	{
		Vector4				 uvTilingAndOffset;
		LinaTexture2DBinding diffuse;
	};

	struct GPUMaterialGUIDisplayTarget
	{
		Vector4				 uvTilingAndOffset;
		LinaTexture2DBinding diffuse;
	};

	struct GPUMaterialGUIHueDisplay
	{
		Vector2 uvContribution;
	};

	struct GPUMaterialGUISDFText
	{
		LinaTexture2DBinding diffuse;
		Vector4				 outlineColor;
		float				 thickness;
		float				 softness;
		float				 outlineThickness;
		float				 outlineSoftness;
	};

	struct GPUMaterialGUIText
	{
		LinaTexture2DBinding diffuse;
	};

	enum class GUISpecialType
	{
		None,
		HorizontalHue,
		VerticalHue,
		ColorWheel,
		Glitch,
		DisplayTarget,
	};

	enum class DisplayChannels
	{
		RGBA,
		R,
		G,
		B,
		A
	};

	LINA_CLASS_BEGIN(DisplayChannels)
	LINA_PROPERTY_STRING(DisplayChannels, 0, "RGBA")
	LINA_PROPERTY_STRING(DisplayChannels, 1, "R")
	LINA_PROPERTY_STRING(DisplayChannels, 2, "G")
	LINA_PROPERTY_STRING(DisplayChannels, 3, "B")
	LINA_PROPERTY_STRING(DisplayChannels, 4, "A")
	LINA_CLASS_END(DisplayChannels)

	struct GUIUserData
	{
		GUISpecialType	specialType			= GUISpecialType::None;
		DisplayChannels displayChannels		= DisplayChannels::RGBA;
		uint32			mipLevel			= 0;
		ResourceID		sampler				= 0;
		float			sdfThickness		= 0.5f;
		float			sdfSoftness			= 0.025f;
		float			sdfOutlineThickness = 0.0f;
		float			sdfOutlineSoftness	= 0.0f;
		Color			sdfOutlineColor		= Color::Black;
		bool			isSingleChannel		= false;
	};

	struct EditorWorldPassViewData
	{
		Matrix4 view;
		Matrix4 proj;
		Matrix4 viewProj;
		Vector4 cameraPositionAndNear;
		Vector4 cameraDirectionAndFar;
		Vector2 size;
		Vector2 mouse;
		Vector4 padding;
	};

	struct GPUDataEditorGUIView
	{
		Matrix4 proj;
	};

	struct GPUDataEditorSwapchainPass
	{
		uint32 textureIndex;
		uint32 samplerIndex;
		uint32 padding0;
		uint32 padding1;
	};

	class EditorGfxHelpers
	{
	public:
		static RenderPassDescription	  GetGUIPassDescription();
		static RenderPassDescription	  GetSwapchainPassDescription();
		static RenderPassDescription	  GetEditorWorldPassDescription();
		static RenderPassDescription	  GetGizmoOrientationPassDescription();
		static LinaGX::DescriptorSetDesc  GetSetDescriptionGUI();
		static LinaGX::DescriptorSetDesc  GetSetDescriptionSwapchain();
		static LinaGX::DescriptorSetDesc  GetSetDescriptionEditorWorldPass();
		static LinaGX::DescriptorSetDesc  GetSetDescriptionGizmoOrientationPass();
		static LinaGX::PipelineLayoutDesc GetPipelineLayoutDescriptionGUI();
		static LinaGX::PipelineLayoutDesc GetPipelineLayoutDescriptionSwapchain();
		static LinaGX::PipelineLayoutDesc GetPipelineLayoutDescriptionEditorWorldPass();
		static LinaGX::PipelineLayoutDesc GetPipelineLayoutDescriptionGizmoOrientationPass();
	};
} // namespace Lina::Editor
