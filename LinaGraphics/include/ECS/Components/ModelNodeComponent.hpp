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

/*
Class: ModelNodeComponent



Timestamp: 12/24/2021 10:20:14 PM
*/

#pragma once

#ifndef ModelNodeComponent_HPP
#define ModelNodeComponent_HPP

// Headers here.
#include "ECS/Component.hpp"
#include "Utility/StringId.hpp"

#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

namespace Lina
{
    namespace Graphics
    {
        class ModelNode;
    }
} // namespace Lina

namespace Lina::ECS
{
    struct ModelNodeComponent : public Component
    {
        Graphics::ModelNode*      m_modelNode = nullptr;
        std::vector<StringIDType> m_materialIDs;

    private:
        friend class cereal::access;

        StringIDType m_modelNodeID = 0;
        StringIDType m_modelID     = 0;

        template <class Archive> void serialize(Archive& archive)
        {
            archive(m_modelNodeID, m_materialIDs, m_isEnabled);
        }
    };

} // namespace Lina::ECS

#endif
