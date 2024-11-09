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

#include "Editor/Undo/UndoActionResourceDirectory.hpp"
#include "Editor/Editor.hpp"
#include "Editor/EditorLocale.hpp"
#include "Editor/Widgets/CommonWidgets.hpp"
#include "Editor/Widgets/Panel/PanelResourceBrowser.hpp"
#include "Editor/Widgets/Panel/PanelFontViewer.hpp"
#include "Editor/Widgets/Panel/PanelMaterialViewer.hpp"
#include "Editor/Widgets/Panel/PanelModelViewer.hpp"
#include "Editor/Widgets/Panel/PanelTextureViewer.hpp"
#include "Editor/Widgets/Panel/PanelPhysicsMaterialViewer.hpp"
#include "Editor/Widgets/Panel/PanelAudioViewer.hpp"
#include "Editor/Widgets/Panel/PanelSamplerViewer.hpp"
#include "Editor/Widgets/Compound/ResourceDirectoryBrowser.hpp"
#include "Editor/IO/ThumbnailGenerator.hpp"
#include "Core/Graphics/Resource/TextureSampler.hpp"
#include "Core/Meta/ProjectData.hpp"
#include "Common/Serialization/Serialization.hpp"
#include "Common/FileSystem/FileSystem.hpp"
#include "Core/Application.hpp"

namespace Lina::Editor
{

	/*
	*** RESOURCE RENAME
	*/

	UndoActionResourceRename* UndoActionResourceRename::Create(Editor* editor, ResourceDirectory* dir, const String& newName)
	{
		UndoActionResourceRename* action = new UndoActionResourceRename();
		action->m_prevName				 = dir->name;
		action->m_newName				 = newName;
		action->m_resourceGUID			 = dir->guid;
		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionResourceRename::Execute(Editor* editor)
	{
		ResourceDirectory* dir = editor->GetProjectManager().GetProjectData()->GetResourceRoot().FindByGUID(m_resourceGUID);
		if (dir == nullptr)
			return;

		dir->name = m_newName;

		editor->GetProjectManager().SaveProjectChanges();

		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::ResourceBrowser, 0);
		if (panel != nullptr)
		{
			PanelResourceBrowser* browser = static_cast<PanelResourceBrowser*>(panel);
			browser->GetBrowser()->RefreshDirectory();
		}

		if (dir->isFolder)
			return;

		// Open, modify, close resource.
		MetaType&	 meta	 = ReflectionSystem::Get().Resolve(dir->resourceTID);
		Resource*	 res	 = static_cast<Resource*>(meta.GetFunction<void*()>("Allocate"_hs)());
		const String resPath = editor->GetProjectManager().GetProjectData()->GetResourcePath(dir->resourceID);
		IStream		 stream	 = Serialization::LoadFromFile(resPath.c_str());
		if (!stream.Empty())
		{
			res->LoadFromStream(stream);
			res->SetName(dir->name);
			res->SaveToFileAsBinary(resPath);
		}
		stream.Destroy();
		meta.GetFunction<void(void*)>("Deallocate"_hs)(res);
	}

	void UndoActionResourceRename::Undo(Editor* editor)
	{
		ResourceDirectory* dir = editor->GetProjectManager().GetProjectData()->GetResourceRoot().FindByGUID(m_resourceGUID);
		if (dir == nullptr)
			return;

		dir->name = m_prevName;

		editor->GetProjectManager().SaveProjectChanges();

		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::ResourceBrowser, 0);
		if (panel != nullptr)
		{
			PanelResourceBrowser* browser = static_cast<PanelResourceBrowser*>(panel);
			browser->GetBrowser()->RefreshDirectory();
		}

		if (dir->isFolder)
			return;
		// Open, modify, close resource.
		MetaType&	 meta	 = ReflectionSystem::Get().Resolve(dir->resourceTID);
		Resource*	 res	 = static_cast<Resource*>(meta.GetFunction<void*()>("Allocate"_hs)());
		const String resPath = editor->GetProjectManager().GetProjectData()->GetResourcePath(dir->resourceID);
		IStream		 stream	 = Serialization::LoadFromFile(resPath.c_str());
		if (!stream.Empty())
		{
			res->LoadFromStream(stream);
			res->SetName(dir->name);
			res->SaveToFileAsBinary(resPath);
		}
		stream.Destroy();
		meta.GetFunction<void(void*)>("Deallocate"_hs)(res);
	}

	/*
	*** SAMPLER DATA CHANGED
	*/

	UndoActionSamplerDataChanged* UndoActionSamplerDataChanged::Create(Editor* editor, ResourceID resourceID, const LinaGX::SamplerDesc& prevDesc)
	{
		UndoActionSamplerDataChanged* action = new UndoActionSamplerDataChanged();
		action->m_resourceID				 = resourceID;
		action->m_prevDesc					 = prevDesc;
		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionSamplerDataChanged::Execute(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::SamplerViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<TextureSampler>(m_resourceID);
		if (res == nullptr)
			return;

		TextureSampler* sampler = static_cast<TextureSampler*>(res);
		res->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(res->GetID()));
		editor->GetProjectManager().ReloadResourceInstances(res);
	}

	void UndoActionSamplerDataChanged::Undo(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::SamplerViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<TextureSampler>(m_resourceID);
		if (res == nullptr)
			return;

		TextureSampler* sampler = static_cast<TextureSampler*>(res);
		sampler->GetDesc()		= m_prevDesc;
		res->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(res->GetID()));
		editor->GetProjectManager().ReloadResourceInstances(res);

		static_cast<PanelSamplerViewer*>(panel)->Rebuild();
	}

	/*
	*** FONT DATA CHANGED
	*/

	UndoActionFontDataChanged* UndoActionFontDataChanged::Create(Editor* editor, ResourceID resourceID, const Font::Metadata& meta)
	{
		UndoActionFontDataChanged* action = new UndoActionFontDataChanged();
		action->m_resourceID			  = resourceID;
		action->m_prevMeta				  = meta;
		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionFontDataChanged::Execute(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::FontViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Font>(m_resourceID);
		if (res == nullptr)
			return;

		EditorTask* task   = editor->GetTaskManager().CreateTask();
		task->ownerWindow  = panel->GetWindow();
		task->title		   = Locale::GetStr(LocaleStr::Saving);
		task->progressText = Locale::GetStr(LocaleStr::Working);

		task->task = [res, editor]() {
			Font* font = static_cast<Font*>(res);
			font->DestroySW();
			font->GenerateSW();
			res->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(res->GetID()));
		};

		task->onComplete = [res, editor]() { editor->GetProjectManager().ReloadResourceInstances(res); };

		editor->GetTaskManager().AddTask(task);
	}

	void UndoActionFontDataChanged::Undo(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::FontViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Font>(m_resourceID);
		if (res == nullptr)
			return;

		EditorTask* task   = editor->GetTaskManager().CreateTask();
		task->ownerWindow  = panel->GetWindow();
		task->title		   = Locale::GetStr(LocaleStr::Saving);
		task->progressText = Locale::GetStr(LocaleStr::Working);

		Font::Metadata prevMeta = m_prevMeta;
		task->task				= [res, editor, prevMeta]() {
			 Font* font		 = static_cast<Font*>(res);
			 font->GetMeta() = prevMeta;
			 font->DestroySW();
			 font->GenerateSW();
			 res->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(res->GetID()));
		};

		task->onComplete = [res, editor]() {
			editor->GetProjectManager().ReloadResourceInstances(res);

			Panel* p = editor->GetWindowPanelManager().FindPanelOfType(PanelType::FontViewer, res->GetID());
			if (p == nullptr)
				return;
			static_cast<PanelFontViewer*>(p)->Rebuild();
		};

		editor->GetTaskManager().AddTask(task);
	}

	/*
	** TEXTURE
	*/

	UndoActionTextureDataChanged* UndoActionTextureDataChanged::Create(Editor* editor, ResourceID resourceID, const Texture::Metadata& meta)
	{
		UndoActionTextureDataChanged* action = new UndoActionTextureDataChanged();
		action->m_resourceID				 = resourceID;
		action->m_prevMeta					 = meta;
		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionTextureDataChanged::Execute(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::TextureViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Texture>(m_resourceID);
		if (res == nullptr)
			return;

		Apply(editor, false);
	}

	void UndoActionTextureDataChanged::Undo(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::TextureViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Texture>(m_resourceID);
		if (res == nullptr)
			return;

		Apply(editor, true);
	}

	void UndoActionTextureDataChanged::Apply(Editor* editor, bool applyMeta)
	{
		Texture* res   = editor->GetApp()->GetResourceManager().GetResource<Texture>(m_resourceID);
		Panel*	 panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::TextureViewer, m_resourceID);

		EditorTask* task   = editor->GetTaskManager().CreateTask();
		task->ownerWindow  = panel->GetWindow();
		task->title		   = Locale::GetStr(LocaleStr::Saving);
		task->progressText = Locale::GetStr(LocaleStr::Working);

		Texture::Metadata prevMeta = m_prevMeta;

		task->task = [editor, res, applyMeta, prevMeta]() {
			if (prevMeta.force8Bit != res->GetMeta().force8Bit)
			{
				ResourceDirectory* dir	= editor->GetProjectManager().GetProjectData()->GetResourceRoot().FindResourceDirectory(res->GetID());
				const String	   path = FileSystem::GetFilePath(editor->GetProjectManager().GetProjectData()->GetPath()) + dir->sourcePathRelativeToProject;

				if (applyMeta)
					res->GetMeta() = prevMeta;

				res->LoadFromFile(path);
			}
			else
			{
				if (applyMeta)
					res->GetMeta() = prevMeta;

				res->VerifyMipmaps();
				res->DetermineFormat();
			}

			res->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(res->GetID()));
		};

		task->onComplete = [editor, res]() {
			editor->GetProjectManager().ReloadResourceInstances(res);

			Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::TextureViewer, res->GetID());
			if (panel == nullptr)
				return;
			static_cast<PanelTextureViewer*>(panel)->UpdateTextureProps();
		};

		editor->GetTaskManager().AddTask(task);
	}

	/*
	** MATERIAL
	*/

	UndoActionMaterialDataChanged* UndoActionMaterialDataChanged::Create(Editor* editor, ResourceID resourceID, const OStream& prevStream)
	{
		UndoActionMaterialDataChanged* action = new UndoActionMaterialDataChanged();
		action->m_resourceID				  = resourceID;
		action->m_prevStream.WriteRaw(prevStream.GetDataRaw(), prevStream.GetCurrentSize());
		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionMaterialDataChanged::Execute(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::MaterialViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Material>(m_resourceID);
		if (res == nullptr)
			return;

		Apply(editor, false);
		static_cast<PanelMaterialViewer*>(panel)->StoreBuffer();
	}

	void UndoActionMaterialDataChanged::Undo(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::MaterialViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Material>(m_resourceID);
		if (res == nullptr)
			return;

		Apply(editor, true);
		static_cast<PanelMaterialViewer*>(panel)->StoreBuffer();
	}

	void UndoActionMaterialDataChanged::Apply(Editor* editor, bool applyMeta)
	{
		Material* mat	= editor->GetApp()->GetResourceManager().GetResource<Material>(m_resourceID);
		Panel*	  panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::MaterialViewer, m_resourceID);

		if (applyMeta)
		{
			IStream stream;
			stream.Create(m_prevStream.GetDataRaw(), m_prevStream.GetCurrentSize());
			mat->LoadFromStream(stream);
			stream.Destroy();
		}

		mat->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(mat->GetID()));
		editor->GetProjectManager().ReloadResourceInstances(mat);

		if (applyMeta)
		{
			static_cast<PanelMaterialViewer*>(panel)->Rebuild();
		}
	}
	/*
	 ** MATERIAL SHADER
	 */

	UndoActionMaterialShaderChanged* UndoActionMaterialShaderChanged::Create(Editor* editor, ResourceID resourceID, ResourceID prevShader, ResourceID newShader)
	{
		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Material>(resourceID);
		if (res == nullptr)
			return nullptr;

		UndoActionMaterialShaderChanged* action = new UndoActionMaterialShaderChanged();
		action->m_resourceID					= resourceID;
		action->m_prevShader					= prevShader;
		action->m_newShader						= newShader;

		editor->GetUndoManager().AddToStack(action);
		return action;
	}

	void UndoActionMaterialShaderChanged::Execute(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::MaterialViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Material>(m_resourceID);
		if (res == nullptr)
			return;

		Material*	 mat  = static_cast<Material*>(res);
		const String path = editor->GetProjectManager().GetProjectData()->GetResourcePath(mat->GetShader());

		if (!FileSystem::FileOrPathExists(path))
			return;

		IStream stream = Serialization::LoadFromFile(path.c_str());
		Shader	sh(0, "");
		sh.LoadFromStream(stream);
		mat->SetShader(&sh);
		stream.Destroy();

		mat->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(mat->GetID()));
		editor->GetProjectManager().ReloadResourceInstances(mat);

		static_cast<PanelMaterialViewer*>(panel)->Rebuild();
	}

	void UndoActionMaterialShaderChanged::Undo(Editor* editor)
	{
		Panel* panel = editor->GetWindowPanelManager().FindPanelOfType(PanelType::MaterialViewer, m_resourceID);
		if (panel == nullptr)
			return;

		Resource* res = editor->GetApp()->GetResourceManager().GetIfExists<Material>(m_resourceID);
		if (res == nullptr)
			return;

		Material*	 mat  = static_cast<Material*>(res);
		const String path = editor->GetProjectManager().GetProjectData()->GetResourcePath(m_prevShader);

		if (!FileSystem::FileOrPathExists(path))
			return;

		IStream stream = Serialization::LoadFromFile(path.c_str());
		Shader	sh(0, "");
		sh.LoadFromStream(stream);
		mat->SetShader(&sh);
		stream.Destroy();

		mat->SaveToFileAsBinary(editor->GetProjectManager().GetProjectData()->GetResourcePath(mat->GetID()));
		editor->GetProjectManager().ReloadResourceInstances(mat);

		static_cast<PanelMaterialViewer*>(panel)->Rebuild();
	}

} // namespace Lina::Editor
