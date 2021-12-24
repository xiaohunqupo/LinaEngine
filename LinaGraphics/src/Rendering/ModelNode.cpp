/* 
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

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

#include "Rendering/ModelNode.hpp"
#include "Utility/AssimpUtility.hpp"
#include "Rendering/SkinnedMesh.hpp"
#include "Rendering/StaticMesh.hpp"
#include "Utility/ModelLoader.hpp"
#include <assimp/scene.h>

namespace Lina::Graphics
{
	ModelNode::~ModelNode()
	{
		for (uint32 i = 0; i < m_meshes.size(); i++)
			delete m_meshes[i];

		m_meshes.clear();
	}

	void ModelNode::FillNodeHierarchy(const aiNode* node, const aiScene* scene)
	{
		m_name = std::string(node->mName.C_Str());
		m_id = StringID(m_name.c_str()).value();
		m_localTransform = AssimpToLinaMatrix(node->mTransformation);

		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
			Mesh* addedMesh = nullptr;

			if (aimesh->HasBones())
			{
				SkinnedMesh* skinned = new SkinnedMesh();
				m_meshes.push_back(skinned);
				addedMesh = skinned;
			}
			else
			{
				StaticMesh* staticMesh = new StaticMesh();
				m_meshes.push_back(staticMesh);
				addedMesh = staticMesh;
			}

			ModelLoader::FillMeshData(aimesh, addedMesh);

			// Finally, construct the vertex array object for the mesh.
			addedMesh->CreateVertexArray(BufferUsage::USAGE_DYNAMIC_DRAW);
		}

		// Recursively fill the other nodes.
		for (uint32 i = 0; i < node->mNumChildren; i++)
		{
			m_children.push_back(ModelNode());
			m_children[m_children.size() - 1].FillNodeHierarchy(node->mChildren[i], scene);
		}
		
	}
}