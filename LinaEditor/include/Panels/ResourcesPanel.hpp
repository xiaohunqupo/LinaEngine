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
Class: ResourcesPanel

Displays the project folders & files, creates functionality to edit, add, remove them.

Timestamp: 6/5/2020 12:54:52 AM
*/

#pragma once

#ifndef ResourcesPanel_HPP
#define ResourcesPanel_HPP

#include "Core/EditorCommon.hpp"
#include "Panels/EditorPanel.hpp"
#include "Utility/UtilityFunctions.hpp"
#include <map>
#include <vector>

namespace Lina::Editor
{

    struct EMenuBarElementClicked;

    class PropertiesPanel;

    class ResourcesPanel : public EditorPanel
    {

    public:
        ResourcesPanel() = default;
        virtual ~ResourcesPanel();

        virtual void Initialize(const char* id, const char* icon) override;
        virtual void Draw() override;

    private:
        void HandleLeftPaneResize(bool canResize);

        void DrawLeftPane();
        void DrawRightPane();

        void DrawContextMenu();
        void DrawFolderMenu(Utility::Folder& folder, bool performFilterCheck);
        void DrawContents(Utility::Folder& folder);
        void DrawFile(Utility::File& file);

    private:
        void OnMenuBarElementClicked(const EMenuBarElementClicked& ev);

    private:
        std::string                  m_searchFilter              = "";
        bool                         m_draggingChildWindowBorder = false;
        bool                         m_showEditorFolders         = true;
        bool                         m_showEngineFolders         = true;
        float                        m_leftPaneWidth             = 280.0f;
        float                        m_leftPaneMinWidth          = 200.0f;
        float                        m_leftPaneMaxWidth          = 500.0f;
        std::vector<Utility::Folder> m_folders;
        Utility::Folder*             m_currentSelectedFolder = nullptr;
        Utility::File*               m_currentSelectedFile   = nullptr;
    };
} // namespace Lina::Editor

#endif
