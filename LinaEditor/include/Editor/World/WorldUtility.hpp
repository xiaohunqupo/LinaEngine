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

#include "Core/Resources/CommonResources.hpp"
#include "Core/World/CommonWorld.hpp"
#include "Common/Math/Vector.hpp"

namespace Lina
{
	class Material;
	class ResourceManagerV2;
	class Model;
	class Entity;
	class EntityWorld;
	struct ResourceDirectory;
} // namespace Lina

namespace Lina::Editor
{
	class Editor;

	class WorldUtility
	{
	public:
		static void			   SetupDefaultSkyMaterial(Material* mat, ResourceManagerV2* rm);
		static Entity*		   AddModelToWorld(EntityID guid, EntityWorld* world, Model* model, const Vector<ResourceID>& materials);
		static void			   LoadModelAndMaterials(Editor* editor, ResourceID model, ResourceID resourceSpace);
		static Vector<Entity*> AddResourcesToWorld(Editor* editor, EntityWorld* world, const Vector<ResourceDirectory*>& dirs, const Vector3& pos);

		static void FixEntityIDsToNew(EntityWorld* world, const Vector<Entity*>& entities);
		static void SaveEntitiesToStream(OStream& stream, EntityWorld* world, const Vector<Entity*>& entities);
		static void LoadEntitiesFromStream(IStream& stream, EntityWorld* world, Vector<Entity*>& outEntities);
		static void DuplicateEntities(Editor* editor, EntityWorld* world, const Vector<Entity*>& srcEntities, Vector<Entity*>& outEntities);
	};
} // namespace Lina::Editor
