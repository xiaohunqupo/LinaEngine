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

#ifndef Shader_HPP
#define Shader_HPP

#include "Resources/Core/IResource.hpp"
#include "Platform/LinaGXIncl.hpp"

namespace Lina
{
	struct ShaderMeta
	{
		bool disableBlend = false;
		bool disableDepth = false;
		bool isFinalPass  = false;
	};

	class Shader : public IResource
	{
	public:
		Shader(ResourceManager* rm, bool isUserManaged, const String& path, StringID sid) : IResource(rm, isUserManaged, path, sid, GetTypeID<Shader>()){};
		virtual ~Shader();

		inline uint32 GetGPUHandle() const
		{
			return m_gpuHandle;
		}

		inline const LinaGX::ShaderLayout& GetLayout() const
		{
			return m_layout;
		}

		inline const LinaGX::ShaderUBO& GetMaterialUBO() const
		{
			return m_materialUBO;
		}

	protected:
		// Inherited via IResource
		virtual void LoadFromFile(const char* path) override;
		virtual void SaveToStream(OStream& stream) override;
		virtual void LoadFromStream(IStream& stream) override;
		virtual void BatchLoaded() override;
		virtual void Flush() override;

	private:
	private:
		uint32											  m_gpuHandle = 0;
		LINAGX_MAP<LinaGX::ShaderStage, LinaGX::DataBlob> m_outCompiledBlobs;
		LinaGX::ShaderLayout							  m_layout		= {};
		LinaGX::ShaderUBO								  m_materialUBO = {};
		ShaderMeta										  m_meta;
	};

} // namespace Lina

#endif
