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

#include "Common/Data/Vector.hpp"
#include "Common/Math/Vector.hpp"
#include "Common/Math/Matrix.hpp"
#include "Common/Data/CommonData.hpp"
#include "Core/Resources/CommonResources.hpp"
#include "Core/World/CommonWorld.hpp"

namespace Lina
{
	class Material;
	class Buffer;

	struct MaterialComparator
	{
		bool operator()(const Material* lhs, const Material* rhs) const;
	};

	enum RenderPassType
	{
		RENDER_PASS_DEFERRED = 0,
		RENDER_PASS_FORWARD,
		RENDER_PASS_UNKNOWN,
	};

	enum MeshFormat
	{
		MESH_FORMAT_STATIC = 0,
		MESH_FORMAT_SKINNED,
		MESH_FORMAT_MAX,
	};

	constexpr const char* RPTypeToString(RenderPassType type)
	{
		switch (type)
		{
		case RENDER_PASS_DEFERRED:
			return "Main";
		case RENDER_PASS_FORWARD:
			return "ForwardTransparency";
		default:
			return "Unknown";
		}
	}

	enum RenderableType
	{
		RenderableSprite	  = 1 << 0,
		RenderableDecal		  = 1 << 1,
		RenderableSky		  = 1 << 2,
		RenderableSkinnedMesh = 1 << 3,
		RenderableStaticMesh  = 1 << 4,
		RenderableParticle	  = 1 << 5,
	};

	struct GPUDataEngineGlobals
	{
		Vector4 deltaElapsed;
	};

	struct GPUDataView
	{
		Matrix4 view;
		Matrix4 proj;
		Matrix4 viewProj;
		Vector4 cameraPositionAndNear;
		Vector4 cameraDirectionAndFar;
		Vector2 size;
		Vector2 mouse;
		Vector4 padding2;
	};

	struct GPUMaterialGUI
	{
		Vector4 color1;
		Vector4 color2;
		Vector4 floatPack1;
		Vector4 floatPack2;
		Vector4 clip;
		Vector2 canvasSize;
		Vector2 padding;
	};

	struct GPUEntity
	{
		Matrix4 model;
	};

	struct GPUDrawArguments
	{
		uint32 constant0;
		uint32 constant1;
		uint32 constant2;
		uint32 constant3;
	};

	struct GPUForwardPassData
	{
		uint32 gBufAlbedo			= 0;
		uint32 gBufPositionMetallic = 0;
		uint32 gBufNormalRoughness	= 0;
		uint32 gBufSampler			= 0;
		uint32 pad0					= 0;
		uint32 pad1					= 0;
		uint32 pad3					= 0;
		uint32 pad4					= 0;
	};

	struct Line3DVertex
	{
		Vector3 position;
		Vector3 nextPosition;
		Vector4 color;
		float	direction;
	};

	struct EntityIdent
	{
		EntityID entityGUID = 0;
		uint32	 uniqueID2	= 0;
		uint32	 uniqueID3	= 0;
		uint32	 uniqueID4	= 0;

		bool operator==(const EntityIdent& other) const
		{
			return entityGUID == other.entityGUID && uniqueID2 == other.uniqueID2 && uniqueID3 == other.uniqueID3 && uniqueID4 == other.uniqueID4;
		}
	};

	struct DrawEntity
	{
		GPUEntity	entity;
		EntityIdent ident;
	};

} // namespace Lina
