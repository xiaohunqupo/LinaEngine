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

#include "PipelineObjects/Pipeline.hpp"
#include "Core/Backend.hpp"
#include "Core/RenderEngine.hpp"
#include "Utility/Vulkan/VulkanUtility.hpp"
#include "PipelineObjects/RenderPass.hpp"
#include "PipelineObjects/CommandBuffer.hpp"
#include "PipelineObjects/PipelineLayout.hpp"
#include "Resource/Shader.hpp"
#include <vulkan/vulkan.h>

namespace Lina::Graphics
{
    void Pipeline::Create()
    {
        VkViewport _viewport = VkViewport{
            .x        = viewport.x,
            .y        = viewport.y,
            .width    = viewport.width,
            .height   = viewport.height,
            .minDepth = viewport.minDepth,
            .maxDepth = viewport.maxDepth,
        };

        VkRect2D _scissor = VkRect2D{
            .offset = VkOffset2D(scissor.pos.x, scissor.pos.y),
            .extent = VkExtent2D(scissor.size.x, scissor.size.y),
        };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext         = nullptr,
            .viewportCount = 1,
            .pViewports    = &_viewport,
            .scissorCount  = 1,
            .pScissors     = &_scissor,
        };

        VkPipelineColorBlendAttachmentState _colorBlendAttachment = VulkanUtility::CreatePipelineBlendAttachmentState();

        // Write to color attachment, no blending, dummy
        VkPipelineColorBlendStateCreateInfo _colorBlending = {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext           = nullptr,
            .logicOpEnable   = VK_FALSE,
            .logicOp         = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments    = &_colorBlendAttachment,
        };

        VkPipelineInputAssemblyStateCreateInfo _inputAssembly = VulkanUtility::CreatePipelineInputAssemblyCreateInfo(topology);
        VkPipelineMultisampleStateCreateInfo   _msaa          = VulkanUtility::CreatePipelineMSAACreateInfo();
        VkPipelineRasterizationStateCreateInfo _raster        = VulkanUtility::CreatePipelineRasterStateCreateInfo(polygonMode, cullMode);
        VkPipelineDepthStencilStateCreateInfo  _depthStencil  = VulkanUtility::CreatePipelineDepthStencilStateCreateInfo(depthTestEnabled, depthWriteEnabled, depthCompareOp);
        // First get vertex input description
        // Then fill binding & description VULKAN structs with this data
        // Use the structs to get input state create info
        const VertexInputDescription              _vertexInputDesc = VulkanUtility::GetVertexDescription();
        Vector<VkVertexInputBindingDescription>   _bindingDescs;
        Vector<VkVertexInputAttributeDescription> _attDescs;
        VulkanUtility::GetDescriptionStructs(_vertexInputDesc, _bindingDescs, _attDescs);
        VkPipelineVertexInputStateCreateInfo _vertexInput = VulkanUtility::CreatePipelineVertexInputStateCreateInfo(_bindingDescs, _attDescs);

        Vector<VkPipelineShaderStageCreateInfo> _shaderStages;
        Vector<VkShaderModule_T*>               addedModules;

        const auto& modules = _shader->GetModules();

        for (auto& [stage, mod] : modules)
        {
            VkShaderModule_T* _mod = mod.ptr;
            addedModules.push_back(_mod);
            VkPipelineShaderStageCreateInfo info = VulkanUtility::CreatePipelineShaderStageCreateInfo(stage, _mod);
            _shaderStages.push_back(info);
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = VkGraphicsPipelineCreateInfo{
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = nullptr,
            .stageCount          = static_cast<uint32>(_shaderStages.size()),
            .pStages             = _shaderStages.data(),
            .pVertexInputState   = &_vertexInput,
            .pInputAssemblyState = &_inputAssembly,
            .pViewportState      = &viewportState,
            .pRasterizationState = &_raster,
            .pMultisampleState   = &_msaa,
            .pDepthStencilState  = &_depthStencil,
            .pColorBlendState    = &_colorBlending,
            .layout              = _layout,
            .renderPass          = _renderPass,
            .subpass             = 0,
            .basePipelineHandle  = VK_NULL_HANDLE,
        };

        VkResult res = vkCreateGraphicsPipelines(Backend::Get()->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, Backend::Get()->GetAllocator(), &_ptr);
        LINA_ASSERT(res == VK_SUCCESS, "[Pipeline] -> Could not create VK pipeline!");

        // We don't need the modules anymore
        _shaderStages.clear();
        _shader->UploadedToPipeline();

        VkPipeline_T* ptr = _ptr;
        RenderEngine::Get()->GetMainDeletionQueue().Push([ptr]() {
            vkDestroyPipeline(Backend::Get()->GetDevice(), ptr, Backend::Get()->GetAllocator());
        });
    }

    Pipeline& Pipeline::SetShader(Shader* shader)
    {
        _shader = shader;
        return *this;
    }

    Pipeline& Pipeline::SetRenderPass(const RenderPass& rp)
    {
        _renderPass = rp._ptr;
        return *this;
    }
    Pipeline& Pipeline::SetLayout(const PipelineLayout& layout)
    {
        _layout = layout._ptr;
        return *this;
    }

    void Pipeline::Bind(const CommandBuffer& cmd, PipelineBindPoint bindpoint)
    {
        vkCmdBindPipeline(cmd._ptr, GetPipelineBindPoint(bindpoint), _ptr);
    }

} // namespace Lina::Graphics
