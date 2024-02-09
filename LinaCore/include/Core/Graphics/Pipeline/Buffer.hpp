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

#include "Common/Data/String.hpp"

namespace LinaGX
{
	class Instance;
	class CommandStream;
	enum class IndexType;
} // namespace LinaGX

namespace Lina
{
	class OStream;
	class IStream;

	class Buffer
	{
	public:
		Buffer()  = default;
		~Buffer() = default;

		void Create(LinaGX::Instance* lgx, uint32 hintFlags, uint32 size, const String& debugName = "GPUBuffer", bool stagingOnly = false);
		void BufferData(size_t padding, uint8* data, size_t size);
		void MemsetMapped(int32 v);
		bool Copy(LinaGX::CommandStream* stream);
		void Destroy();
		void SaveToStream(OStream& stream) const;
		void LoadFromStream(LinaGX::Instance* lgx, IStream& stream);
		void BindVertex(LinaGX::CommandStream* stream, uint32 size);
		void BindIndex(LinaGX::CommandStream* stream, LinaGX::IndexType indexType);

		inline uint32 GetGPUResource() const
		{
			return m_residesInGPU ? m_gpu : m_staging;
		}

	private:
		static uint64 s_usedCPUVisibleGPUMemory;

		bool			  m_stagingOnly				= false;
		LinaGX::Instance* m_lgx						= nullptr;
		uint32			  m_staging					= 0;
		uint32			  m_gpu						= 0;
		uint32			  m_size					= 0;
		uint8*			  m_mapped					= nullptr;
		uint32			  m_hintFlags				= 0;
		bool			  m_isCPUVisibleGPUResource = false;
		bool			  m_residesInGPU			= false;
		bool			  m_bufferChanged			= false;
	};
} // namespace Lina
