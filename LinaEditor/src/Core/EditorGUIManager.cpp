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

#include "Core/EditorGUIManager.hpp"
#include "Core/EditorCommon.hpp"
#include "Core/GUINode.hpp"

namespace Lina::Editor
{
    EditorGUIManager* EditorGUIManager::s_instance = nullptr;

    void EditorGUIManager::Initialize()
    {
        s_instance = this;
        LoadRoot();
    }

    void EditorGUIManager::LoadRoot()
    {
        m_rootNode       = new GUI::GUINode();
        m_rootNode->name = "Root";
        m_rootNode->AddParentConstraint(GUI::ParentConstraintType::SizeX, 1.0f)->AddParentConstraint(GUI::ParentConstraintType::SizeY, 1.0f);
        m_rootNode->AddParentConstraint(GUI::ParentConstraintType::PosX, 0.5f)->AddParentConstraint(GUI::ParentConstraintType::PosY, 0.5f);

        m_surfaceTop = new SurfaceTop();
        m_surfaceTop->Setup();
        m_rootNode->AddChild(m_surfaceTop);
    }

    void EditorGUIManager::Shutdown()
    {
        delete m_rootNode;
    }

    void EditorGUIManager::Draw()
    {
        m_rootNode->Draw();
    }

} // namespace Lina::Editor
