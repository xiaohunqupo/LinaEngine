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

#include "PipelineObjects/Buffer.hpp"
#include "Core/Backend.hpp"
#include "Core/RenderEngine.hpp"
#include "Utility/Vulkan/vk_mem_alloc.h"
#include <vulkan/vulkan.h>

namespace Lina::Graphics
{
    void Buffer::Create()
    {
        VkBufferCreateInfo bufferInfo = VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .size  = size,
            .usage = GetBufferUsageFlags(bufferUsage),
        };

        VmaAllocationCreateInfo vmaallocInfo = {
            .usage = GetMemoryUsageFlags(memoryUsage),
        };

        VkResult result = vmaCreateBuffer(Backend::Get()->GetVMA(), &bufferInfo, &vmaallocInfo, &_ptr, &_allocation, nullptr);
        LINA_ASSERT(result == VK_SUCCESS, "[Buffer] -> Could not create Buffer!");
    }

    void Buffer::Destroy()
    {
        vmaDestroyBuffer(Backend::Get()->GetVMA(), _ptr, _allocation);
    }

    void Buffer::CopyInto(const void* src, uint32 size)
    {
        void* data;
        vmaMapMemory(Backend::Get()->GetVMA(), _allocation, &data);
        MEMCPY(data, src, size);
        vmaUnmapMemory(Backend::Get()->GetVMA(), _allocation);
    }

} // namespace Lina::Graphics
