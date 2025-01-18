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

#include "Common/StringID.hpp"
#include "Common/Data/String.hpp"
#include "Common/Data/HashSet.hpp"
#include "Common/Common.hpp"
#include "Common/Data/HashSet.hpp"

namespace Lina
{
	class Resource;

#define RESOURCE_ID_ENGINE_SPACE	 UINT64_MAX - 2000
#define RESOURCE_ID_CUSTOM_SPACE	 UINT64_MAX - 10000
#define RESOURCE_ID_CUSTOM_SPACE_MAX RESOURCE_ID_ENGINE_SPACE
#define RESOURCE_ID_MAX				 RESOURCE_ID_CUSTOM_SPACE - 1

#define ENGINE_SHADER_LIGHTING_QUAD_ID		RESOURCE_ID_ENGINE_SPACE
#define ENGINE_SHADER_WORLD_DEBUG_LINE_ID	RESOURCE_ID_ENGINE_SPACE + 1
#define ENGINE_SHADER_LIGHTING_QUAD_PATH	"Resources/Core/Shaders/Lighting/LightingQuad.linashader"
#define ENGINE_SHADER_WORLD_DEBUG_LINE_PATH "Resources/Core/Shaders/World/DebugLine.linashader"

#define RESOURCE_ID_ENGINE_MODELS							RESOURCE_ID_ENGINE_SPACE + 100
#define RESOURCE_ID_ENGINE_TEXTURESRESOURCE_ID_ENGINE_SPACE +200

#define ENGINE_MODEL_CUBE_ID	  RESOURCE_ID_ENGINE_MODELS
#define ENGINE_MODEL_CYLINDER_ID  RESOURCE_ID_ENGINE_MODELS + 1
#define ENGINE_MODEL_SPHERE_ID	  RESOURCE_ID_ENGINE_MODELS + 2
#define ENGINE_MODEL_SKYCUBE_ID	  RESOURCE_ID_ENGINE_MODELS + 3
#define ENGINE_MODEL_PLANE_ID	  RESOURCE_ID_ENGINE_MODELS + 4
#define ENGINE_MODEL_SKYSPHERE_ID RESOURCE_ID_ENGINE_MODELS + 5
#define ENGINE_MODEL_CAPSULE_ID	  RESOURCE_ID_ENGINE_MODELS + 6
#define ENGINE_MODEL_QUAD_ID	  RESOURCE_ID_ENGINE_MODELS + 7

#define ENGINE_MODEL_CUBE_PATH		"Resources/Editor/Models/Cube.glb"
#define ENGINE_MODEL_CYLINDER_PATH	"Resources/Editor/Models/Cylinder.glb"
#define ENGINE_MODEL_SPHERE_PATH	"Resources/Editor/Models/Sphere.glb"
#define ENGINE_MODEL_SKYCUBE_PATH	"Resources/Editor/Models/SkyCube.glb"
#define ENGINE_MODEL_PLANE_PATH		"Resources/Editor/Models/Plane.glb"
#define ENGINE_MODEL_SKYSPHERE_PATH "Resources/Editor/Models/SkySphere.glb"
#define ENGINE_MODEL_CAPSULE_PATH	"Resources/Editor/Models/Capsule.glb"
#define ENGINE_MODEL_QUAD_PATH		"Resources/Editor/Models/Quad.glb"

#define ENGINE_TEXTURE_EMPTY_ALBEDO_PATH			 "Resources/Editor/Textures/EmptyAlbedo.png"
#define ENGINE_TEXTURE_EMPTY_NORMAL_PATH			 "Resources/Editor/Textures/EmptyNormal.png"
#define ENGINE_TEXTURE_EMPTY_METALLIC_ROUGHNESS_PATH "Resources/Editor/Textures/EmptyMetallicRoughness.png"
#define ENGINE_TEXTURE_EMPTY_AO_PATH				 "Resources/Editor/Textures/EmptyAO.png"
#define ENGINE_TEXTURE_EMPTY_EMISSIVE_PATH			 "Resources/Editor/Textures/EmptyEmissive.png"

#define ENGINE_TEXTURE_EMPTY_ALBEDO_ID			   RESOURCE_ID_ENGINE_TEXTURES + 0
#define ENGINE_TEXTURE_EMPTY_NORMAL_ID			   RESOURCE_ID_ENGINE_TEXTURES + 1
#define ENGINE_TEXTURE_EMPTY_METALLIC_ROUGHNESS_ID RESOURCE_ID_ENGINE_TEXTURES + 2
#define ENGINE_TEXTURE_EMPTY_AO_ID				   RESOURCE_ID_ENGINE_TEXTURES + 3
#define ENGINE_TEXTURE_EMPTY_EMISSIVE_ID		   RESOURCE_ID_ENGINE_TEXTURES + 4

	enum class PackageType
	{
		Default,
		Package1,
		Package2,
		PackageLevels,
	};

	enum ResourceFlags
	{
	};

	struct ResourceDef
	{
		ResourceID id		  = 0;
		String	   name		  = "";
		TypeID	   tid		  = 0;
		OStream	   customMeta = {};

		bool operator==(const ResourceDef& other) const
		{
			return id == other.id;
		}
	};

	struct ResourceDefHash
	{
		std::size_t operator()(const ResourceDef& s) const noexcept
		{
			return std::hash<ResourceID>{}(s.id);
		}
	};

	struct ResourceLoadTask
	{
		Taskflow		  tf;
		Vector<Resource*> resources;
		Atomic<bool>	  isCompleted = false;
		int32			  id		  = 0;
		uint64			  startTime	  = 0;
		uint64			  endTime	  = 0;
	};

	enum class ResourceType
	{
		ExternalSource,
		EngineCreated,
	};

	typedef HashSet<ResourceDef, ResourceDefHash> ResourceDefinitionList;
	typedef HashSet<Resource*>					  ResourceList;

} // namespace Lina
