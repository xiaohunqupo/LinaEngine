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

#include "Editor/Widgets/Panel/PanelFontViewer.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Widgets/CommonWidgets.hpp"
#include "Editor/Actions/EditorActionResources.hpp"

#include "Core/GUI/Widgets/WidgetManager.hpp"
#include "Core/GUI/Widgets/Layout/FoldLayout.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/Application.hpp"
#include "Core/Meta/ProjectData.hpp"

namespace Lina::Editor
{

	void PanelFontViewer::Construct()
	{
		PanelResourceViewer::Construct();
		m_fontDisplay = m_manager->Allocate<Text>("Display");
		m_fontDisplay->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
		m_resourceBG->AddChild(m_fontDisplay);
	}

	void PanelFontViewer::Initialize()
	{
		PanelResourceViewer::Initialize();

		if (!m_resource)
			return;

		Font* font									  = static_cast<Font*>(m_resource);
		m_fontDisplay->GetProps().font				  = font->GetID();
		m_fontDisplay->GetProps().valuePtr			  = &m_displayString;
		m_fontDisplay->GetProps().fetchWrapFromParent = true;
		m_fontDisplay->GetProps().isDynamic			  = true;

		StoreEditorActionBuffer();
		UpdateResourceProperties();
		RebuildContents();
	}

	void PanelFontViewer::StoreEditorActionBuffer()
	{
		Font* font	 = static_cast<Font*>(m_resource);
		m_storedMeta = font->GetMeta();
	}

	void PanelFontViewer::UpdateResourceProperties()
	{
		Font* font = static_cast<Font*>(m_resource);
		m_fontName = font->GetName();
	}

	void PanelFontViewer::RebuildContents()
	{
		m_inspector->DeallocAllChildren();
		m_inspector->RemoveAllChildren();

		Font* font = static_cast<Font*>(m_resource);

		CommonWidgets::BuildClassReflection(m_inspector, this, ReflectionSystem::Get().Resolve<PanelFontViewer>(), [this](const MetaType& meta, FieldBase* field) { m_fontDisplay->GetProps().textScale = m_scale; });

		CommonWidgets::BuildClassReflection(
			m_inspector, &font->GetMeta(), ReflectionSystem::Get().Resolve<Font::Metadata>(), [this, font](const MetaType& meta, FieldBase* field) { EditorActionResourceFont::Create(m_editor, font->GetID(), m_storedMeta, font->GetMeta()); });

		if (m_previewOnly)
			DisableRecursively(m_inspector);
	}

} // namespace Lina::Editor
