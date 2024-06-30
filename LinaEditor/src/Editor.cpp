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

#include "Editor/Editor.hpp"
#include "Core/Application.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Renderers/SurfaceRenderer.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/Meta/ProjectData.hpp"
#include "Editor/Widgets/Screens/SplashScreen.hpp"
#include "Editor/Widgets/Docking/DockArea.hpp"
#include "Editor/Widgets/Docking/DockArea.hpp"
#include "Editor/Widgets/Panel/PanelFactory.hpp"
#include "Editor/Widgets/Panel/Panel.hpp"
#include "Editor/Widgets/EditorRoot.hpp"
#include "Editor/Widgets/Popups/ProjectSelector.hpp"
#include "Editor/Widgets/Popups/GenericPopup.hpp"
#include "Editor/Widgets/Testbed.hpp"
#include "Editor/Widgets/DockTestbed.hpp"
#include "Editor/Widgets/CommonWidgets.hpp"
#include "Editor/Widgets/Gizmo/Gizmo.hpp"
#include "Editor/EditorLocale.hpp"
#include "Common/FileSystem/FileSystem.hpp"
#include "Common/Serialization/Serialization.hpp"
#include "Common/Math/Math.hpp"
#include "Core/CommonCore.hpp"
#include "Core/Graphics/Resource/GUIWidget.hpp"
#include "Core/Graphics/Resource/Font.hpp"
#include "Core/Graphics/Resource/Texture.hpp"
#include "Core/Graphics/Resource/Model.hpp"
#include "Editor/Graphics/WorldRendererExtEditor.hpp"

#include <LinaGX/Core/InputMappings.hpp>

namespace Lina
{
	SystemInitializationInfo Lina_GetInitInfo()
	{
		LinaGX::MonitorInfo monitor = LinaGX::Window::GetPrimaryMonitorInfo();

		const uint32 w = monitor.size.x / 4;
		const uint32 h = static_cast<uint32>(static_cast<float>(w) * (static_cast<float>(monitor.size.y) / static_cast<float>(monitor.size.x)));

		LinaGX::VSyncStyle vsync;
		vsync.dx12Vsync	  = LinaGX::DXVsync::None;
		vsync.vulkanVsync = LinaGX::VKVsync::None;

		return SystemInitializationInfo{
			.appName					 = "Lina Editor",
			.windowWidth				 = w,
			.windowHeight				 = h,
			.windowStyle				 = LinaGX::WindowStyle::BorderlessApplication,
			.vsyncStyle					 = vsync,
			.allowTearing				 = true,
			.appDelegate				 = new Lina::Editor::Editor(),
			.resourceManagerUseMetacache = false,
			.clearColor					 = Theme::GetDef().background0,
		};
	}
} // namespace Lina

namespace Lina::Editor
{
	Editor* Editor::s_editor = nullptr;

	bool Editor::FillResourceCustomMeta(StringID sid, OStream& stream)
	{
		if (sid == DEFAULT_FONT_SID)
		{
			Font::Metadata customMeta = {
				.points = {{.size = 14, .dpiLimit = 10.1f}, {.size = 14, .dpiLimit = 1.8f}, {.size = 14, .dpiLimit = 10.0f}},
				.isSDF	= false,

			};
			customMeta.SaveToStream(stream);
			return true;
		}
		// NOTE: 160, 380 is the glyph range for nunito sans

		if (sid == DEFAULT_TEXTURE_CHECKERED_SID || sid == DEFAULT_TEXTURE_LINALOGO)
		{
			Texture::Metadata meta = {
				.samplerSID = DEFAULT_SAMPLER_GUI_SID,
			};

			meta.SaveToStream(stream);
			return true;
		}

		if (sid == DEFAULT_SHADER_GUI_SID)
		{
			Shader::Metadata meta;

			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable = false,
				.depthTest	  = false,
				.depthWrite	  = false,
				.depthFormat  = LinaGX::Format::UNDEFINED,
				.targets	  = {{.format = DEFAULT_RT_FORMAT}},
				.cullMode	  = LinaGX::CullMode::None,
				.frontFace	  = LinaGX::FrontFace::CCW,
			};

			meta.variants["Swapchain"_hs] = ShaderVariant{
				.blendDisable = false,
				.depthTest	  = false,
				.depthWrite	  = false,
				.depthFormat  = LinaGX::Format::UNDEFINED,
				.targets	  = {{.format = DEFAULT_SWAPCHAIN_FORMAT}},
				.cullMode	  = LinaGX::CullMode::None,
				.frontFace	  = LinaGX::FrontFace::CCW,
			};

			meta.drawIndirectEnabled		  = true;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::Gui;
			meta.descriptorSetAllocationCount = 1;
			meta.SaveToStream(stream);
			return true;
		}

		if (sid == DEFAULT_SHADER_GUI3D_SID)
		{
			Shader::Metadata meta;

			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable		 = false,
				.blendSrcFactor		 = LinaGX::BlendFactor::SrcAlpha,
				.blendDstFactor		 = LinaGX::BlendFactor::OneMinusSrcAlpha,
				.blendColorOp		 = LinaGX::BlendOp::Add,
				.blendSrcAlphaFactor = LinaGX::BlendFactor::One,
				.blendDstAlphaFactor = LinaGX::BlendFactor::One,
				.blendAlphaOp		 = LinaGX::BlendOp::Add,
				.depthTest			 = true,
				.depthWrite			 = true,
				.targets			 = {{.format = DEFAULT_RT_FORMAT}},
				.cullMode			 = LinaGX::CullMode::None,
				.frontFace			 = LinaGX::FrontFace::CCW,
			};

			meta.drawIndirectEnabled		  = true;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::ForwardTransparency;
			meta.descriptorSetAllocationCount = 1;
			meta.SaveToStream(stream);
			return true;
		}

		if (sid == DEFAULT_SHADER_OBJECT_SID)
		{
			Shader::Metadata meta;
			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable = true,
				.depthTest	  = true,
				.depthWrite	  = true,
				.targets	  = {{.format = DEFAULT_RT_FORMAT}, {.format = DEFAULT_RT_FORMAT}, {.format = DEFAULT_RT_FORMAT}},
				.cullMode	  = LinaGX::CullMode::Back,
				.frontFace	  = LinaGX::FrontFace::CW,
			};

			meta.descriptorSetAllocationCount = 1;
			meta.drawIndirectEnabled		  = true;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::Main;
			meta.materialSize				  = sizeof(GPUMaterialDefaultObject);
			meta.SaveToStream(stream);
			return true;
		}

		if (sid == DEFAULT_SHADER_DEFERRED_LIGHTING_SID)
		{
			Shader::Metadata meta;
			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable = false,
				.depthTest	  = false,
				.depthWrite	  = false,
				.targets	  = {{.format = DEFAULT_RT_FORMAT}},
				.cullMode	  = LinaGX::CullMode::None,
				.frontFace	  = LinaGX::FrontFace::CW,
			};
			meta.descriptorSetAllocationCount = 1;
			meta.drawIndirectEnabled		  = false;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::Lighting;
			meta.materialSize				  = 0;
			meta.SaveToStream(stream);
			return true;
		}

		if (sid == DEFAULT_SHADER_SKY_SID)
		{
			Shader::Metadata meta;
			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable	   = true,
				.depthTest		   = true,
				.depthWrite		   = false,
				.targets		   = {{.format = DEFAULT_RT_FORMAT}},
				.depthOp		   = LinaGX::CompareOp::Equal,
				.cullMode		   = LinaGX::CullMode::Back,
				.frontFace		   = LinaGX::FrontFace::CW,
				.depthBiasEnable   = true,
				.depthBiasConstant = 5.0f,
			};

			meta.descriptorSetAllocationCount = 1;
			meta.drawIndirectEnabled		  = false;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::Lighting;
			meta.materialSize				  = sizeof(GPUMaterialDefaultSky);
			meta.SaveToStream(stream);
			return true;
		}

		if (sid == ICON_FONT_SID)
		{
			Font::Metadata customMeta = {
				.points = {{.size = 32, .dpiLimit = 10.0f}},
				.isSDF	= true,
			};
			customMeta.SaveToStream(stream);
			return true;
		}

		if (sid == ALT_FONT_SID || sid == ALT_FONT_BOLD_SID)
		{
			Font::Metadata customMeta = {
				.points = {{.size = 14, .dpiLimit = 1.1f}, {.size = 14, .dpiLimit = 1.8f}, {.size = 16, .dpiLimit = 10.0f}},
				.isSDF	= false,
			};
			customMeta.SaveToStream(stream);
			return true;
		}

		if (sid == "Resources/Editor/Shaders/Lines.linashader"_hs)
		{
			Shader::Metadata meta;
			meta.variants["RenderTarget"_hs] = ShaderVariant{
				.blendDisable = false,

				.depthTest	= true,
				.depthWrite = true,
				.targets	= {{.format = DEFAULT_RT_FORMAT}},
				.cullMode	= LinaGX::CullMode::None,
				.frontFace	= LinaGX::FrontFace::CCW,
			};

			meta.drawIndirectEnabled		  = true;
			meta.renderPassDescriptorType	  = RenderPassDescriptorType::ForwardTransparency;
			meta.descriptorSetAllocationCount = 1;
			meta.SaveToStream(stream);
			return true;
		}
		return false;
	}

	void Editor::PreInitialize()
	{
		m_rm = m_app->GetSystem()->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		m_rm->AddListener(this);

		const String metacachePath = FileSystem::GetUserDataFolder() + "Editor/ResourceCache/";
		if (!FileSystem::FileOrPathExists(metacachePath))
			FileSystem::CreateFolderInPath(metacachePath);

		// Absolute priority resources.
		Vector<ResourceIdentifier> priorityResources;
		const String			   fullPathBase = FileSystem::GetRunningDirectory() + "/";
		priorityResources.push_back({fullPathBase + DEFAULT_SHADER_GUI_PATH, DEFAULT_SHADER_GUI_PATH, GetTypeID<Shader>()});
		priorityResources.push_back({fullPathBase + DEFAULT_SHADER_GUI3D_PATH, DEFAULT_SHADER_GUI3D_PATH, GetTypeID<Shader>()});
		priorityResources.push_back({fullPathBase + DEFAULT_FONT_PATH, DEFAULT_FONT_PATH, GetTypeID<Font>()});
		priorityResources.push_back({fullPathBase + DEFAULT_SHADER_OBJECT_PATH, DEFAULT_SHADER_OBJECT_PATH, GetTypeID<Shader>()});
		priorityResources.push_back({fullPathBase + DEFAULT_SHADER_SKY_PATH, DEFAULT_SHADER_SKY_PATH, GetTypeID<Shader>()});
		priorityResources.push_back({fullPathBase + DEFAULT_TEXTURE_CHECKERED_DARK_PATH, DEFAULT_TEXTURE_CHECKERED_DARK_PATH, GetTypeID<Texture>()});
		priorityResources.push_back({fullPathBase + "Resources/Editor/Textures/LinaLogoTitle.png", "Resources/Editor/Textures/LinaLogoTitle.png", GetTypeID<Texture>()});
		priorityResources.push_back({fullPathBase + ICON_FONT_PATH, ICON_FONT_PATH, GetTypeID<Font>()});
		m_rm->LoadResourcesFromFile(-1, priorityResources, metacachePath);
		m_rm->WaitForAll();
	}

	void Editor::Initialize()
	{
		const String metacachePath = FileSystem::GetUserDataFolder() + "Editor/ResourceCache/";

		s_editor							  = this;
		Theme::GetDef().iconFont			  = ICON_FONT_SID;
		Theme::GetDef().defaultFont			  = DEFAULT_FONT_SID;
		Theme::GetDef().altFont				  = ALT_FONT_BOLD_SID;
		Theme::GetDef().iconDropdown		  = ICON_ARROW_DOWN;
		Theme::GetDef().iconSliderHandle	  = ICON_CIRCLE_FILLED;
		Theme::GetDef().iconColorWheelPointer = ICON_CIRCLE;

		m_worldManager = m_app->GetSystem()->CastSubsystem<WorldManager>(SubsystemType::WorldManager);
		m_worldManager->AddListener(this);
		m_gfxManager = m_app->GetSystem()->CastSubsystem<GfxManager>(SubsystemType::GfxManager);
		m_gfxManager->CreateRendererPool("WorldRenderers"_hs, 0, false);
		m_gfxManager->CreateRendererPool("SurfaceRenderers"_hs, 1, true);

		m_fileManager.Initialize(this);
		m_atlasManager.Initialize(this);
		m_windowPanelManager.Initialize(this);

		m_mainWindow		   = m_gfxManager->GetApplicationWindow(LINA_MAIN_SWAPCHAIN);
		m_primaryWidgetManager = &m_windowPanelManager.GetSurfaceRenderer(LINA_MAIN_SWAPCHAIN)->GetWidgetManager();

		// Push splash
		Widget*		  root	 = m_primaryWidgetManager->GetRoot();
		SplashScreen* splash = root->GetWidgetManager()->Allocate<SplashScreen>();
		splash->Initialize();
		root->AddChild(splash);

		// Load editor settings or create and save empty if non-existing.
		const String userDataFolder = FileSystem::GetUserDataFolder() + "Editor/";
		const String settingsPath	= userDataFolder + "EditorSettings.linameta";
		m_settings.SetPath(settingsPath);
		if (!FileSystem::FileOrPathExists(userDataFolder))
			FileSystem::CreateFolderInPath(userDataFolder);

		if (FileSystem::FileOrPathExists(settingsPath))
			m_settings.LoadFromFile();
		else
			m_settings.SaveToFile();

		// Async load core resources.
		Vector<ResourceIdentifier> list;
		const String			   fullPathBase = FileSystem::GetRunningDirectory() + "/";
		list.push_back({fullPathBase + DEFAULT_SHADER_DEFERRED_LIGHTING_PATH, DEFAULT_SHADER_DEFERRED_LIGHTING_PATH, GetTypeID<Shader>()});
		list.push_back({fullPathBase + "Resources/Core/Models/Plane.glb", "Resources/Core/Models/Plane.glb", GetTypeID<Model>()});
		list.push_back({fullPathBase + "Resources/Core/Models/Cube.glb", "Resources/Core/Models/Cube.glb", GetTypeID<Model>()});
		list.push_back({fullPathBase + "Resources/Core/Models/Sphere.glb", "Resources/Core/Models/Sphere.glb", GetTypeID<Model>()});
		list.push_back({fullPathBase + "Resources/Core/Models/SkyCube.glb", "Resources/Core/Models/SkyCube.glb", GetTypeID<Model>()});
		list.push_back({fullPathBase + DEFAULT_TEXTURE_CHECKERED_PATH, DEFAULT_TEXTURE_CHECKERED_PATH, GetTypeID<Texture>()});
		list.push_back({fullPathBase + ALT_FONT_PATH, ALT_FONT_PATH, GetTypeID<Font>()});
		list.push_back({fullPathBase + ALT_FONT_BOLD_PATH, ALT_FONT_BOLD_PATH, GetTypeID<Font>()});
		list.push_back({fullPathBase + "Resources/Editor/Textures/LinaLogoTitleHorizontal.png", "Resources/Editor/Textures/LinaLogoTitleHorizontal.png", GetTypeID<Texture>()});
		list.push_back({fullPathBase + "Resources/Editor/Shaders/Lines.linashader", "Resources/Editor/Shaders/Lines.linashader", GetTypeID<Shader>()});
		m_rm->LoadResourcesFromFile(RTID_CORE_RES, list, metacachePath);
	}

	void Editor::PreTick()
	{
		m_windowPanelManager.PreTick();
	}

	void Editor::CoreResourcesLoaded()
	{
		// Remove splash
		Widget* root   = m_primaryWidgetManager->GetRoot();
		Widget* splash = root->GetChildren().at(0);
		root->RemoveChild(splash);
		root->GetWidgetManager()->Deallocate(splash);

		// Resize window to work dims.
		m_mainWindow->SetPosition(m_mainWindow->GetMonitorInfoFromWindow().workTopLeft);
		m_mainWindow->AddSizeRequest(m_mainWindow->GetMonitorWorkSize());

		// Insert editor root.
		m_editorRoot = root->GetWidgetManager()->Allocate<EditorRoot>("EditorRoot");
		m_editorRoot->Initialize();
		root->AddChild(m_editorRoot);

		// We either load the last project, or load project selector in forced-mode.
		if (FileSystem::FileOrPathExists(m_settings.GetLastProjectPath()))
			OpenProject(m_settings.GetLastProjectPath());
		else
			OpenPopupProjectSelector(false);

		m_settings.GetLayout().ApplyStoredLayout();
	}

	void Editor::PreShutdown()
	{
		m_rm->RemoveListener(this);
		m_atlasManager.Shutdown();
		DestroyWorldRenderer(m_worldManager->GetMainWorld());
		m_worldManager->UninstallMainWorld();
		m_worldManager->RemoveListener(this);
		m_fileManager.Shutdown();
		m_windowPanelManager.Shutdown();
		m_settings.SaveToFile();
		RemoveCurrentProject();
	}

	void Editor::OpenPopupProjectSelector(bool canCancel, bool openCreateFirst)
	{
		ProjectSelector* projectSelector = m_primaryWidgetManager->Allocate<ProjectSelector>("ProjectSelector");
		projectSelector->SetCancellable(canCancel);
		projectSelector->SetTab(openCreateFirst ? 0 : 1);
		projectSelector->Initialize();

		// When we select a project to open -> ask if we want to save current one if its dirty.
		projectSelector->GetProps().onProjectOpened = [this](const String& location) {
			if (m_currentProject && m_currentProject->GetIsDirty())
			{
				GenericPopup* popup = CommonWidgets::ThrowGenericPopup(Locale::GetStr(LocaleStr::UnfinishedWorkTitle), Locale::GetStr(LocaleStr::UnfinishedWorkDesc), m_primaryWidgetManager->GetRoot());

				m_primaryWidgetManager->AddToForeground(popup);

				popup->AddButton({.text = Locale::GetStr(LocaleStr::Yes), .onClicked = [location, popup, this]() {
									  SaveProjectChanges();
									  RemoveCurrentProject();
									  OpenProject(location);
									  m_primaryWidgetManager->RemoveFromForeground(popup);
									  m_primaryWidgetManager->Deallocate(popup);
								  }});

				popup->AddButton({.text = Locale::GetStr(LocaleStr::No), .onClicked = [location, popup, this]() {
									  RemoveCurrentProject();
									  OpenProject(location);
									  m_primaryWidgetManager->RemoveFromForeground(popup);
									  m_primaryWidgetManager->Deallocate(popup);
								  }});
			}
			else
			{
				RemoveCurrentProject();
				OpenProject(location);
			}
		};

		projectSelector->GetProps().onProjectCreated = [&](const String& path) {
			RemoveCurrentProject();

			if (m_currentProject && m_currentProject->GetIsDirty())
			{
				GenericPopup* popup = CommonWidgets::ThrowGenericPopup(Locale::GetStr(LocaleStr::UnfinishedWorkTitle), Locale::GetStr(LocaleStr::UnfinishedWorkDesc), m_primaryWidgetManager->GetRoot());

				// Save first then create & open.
				popup->AddButton({.text = Locale::GetStr(LocaleStr::Yes), .onClicked = [&]() {
									  SaveProjectChanges();
									  CreateEmptyProjectAndOpen(path);
								  }});

				// Create & open without saving
				popup->AddButton({.text = Locale::GetStr(LocaleStr::No), .onClicked = [&]() { CreateEmptyProjectAndOpen(path); }});
			}
			else
				CreateEmptyProjectAndOpen(path);
		};

		m_primaryWidgetManager->AddToForeground(projectSelector);
		m_primaryWidgetManager->SetForegroundDim(0.5f);
	}

	void Editor::SaveProjectChanges()
	{
		SaveSettings();

		m_currentProject->SetDirty(false);
		m_currentProject->SaveToFile();
	}

	void Editor::CreateEmptyProjectAndOpen(const String& path)
	{
		ProjectData dummy;
		dummy.SetPath(path);
		dummy.SetProjectName(FileSystem::GetFilenameOnlyFromPath(path));
		dummy.SaveToFile();
		OpenProject(path);
	}

	void Editor::RemoveCurrentProject()
	{
		if (m_currentProject == nullptr)
			return;

		delete m_currentProject;
		m_currentProject = nullptr;
	}

	void Editor::OpenProject(const String& projectFile)
	{
		LINA_ASSERT(m_currentProject == nullptr, "");
		m_currentProject = new ProjectData();
		m_currentProject->SetPath(projectFile);
		m_currentProject->LoadFromFile();
		m_editorRoot->SetProjectName(m_currentProject->GetProjectName());

		m_settings.SetLastProjectPath(projectFile);
		m_settings.SaveToFile();
		m_fileManager.SetProjectDirectory(FileSystem::GetFilePath(projectFile));
		m_fileManager.RefreshResources();

		const String& lastWorldPath = m_settings.GetLastWorldAbsPath();
		if (true || FileSystem::FileOrPathExists(lastWorldPath))
		{
			m_worldManager->InstallWorld(lastWorldPath);
			CreateWorldRenderer(m_worldManager->GetMainWorld());
		}
	}

	void Editor::RequestExit()
	{
		SaveSettings();
		m_app->Quit();
	}

	void Editor::SaveSettings()
	{
		auto* loadedWorld = m_app->GetSystem()->CastSubsystem<WorldManager>(SubsystemType::WorldManager)->GetMainWorld();
		if (loadedWorld)
			m_settings.SetLastWorldAbsPath(loadedWorld->GetPath());

		m_settings.GetLayout().StoreLayout();
		m_settings.SaveToFile();
	}

	void Editor::OnWorldInstalled(EntityWorld* world)
	{
		// auto* wr = world->GetRenderer();
		//
		// if (wr != nullptr)
		// 	wr->AddExtension(new WorldRendererExtEditor());
	}

	void Editor::CreateWorldRenderer(EntityWorld* world)
	{
		WorldRenderer* wr = new WorldRenderer(m_gfxManager, world, m_gfxManager->GetApplicationWindow(LINA_MAIN_SWAPCHAIN)->GetSize());
		m_gfxManager->AddRenderer(wr, "WorldRenderers"_hs);
		m_worldRenderers[world] = wr;
	}

	void Editor::DestroyWorldRenderer(EntityWorld* world)
	{
		auto it = m_worldRenderers.find(world);
		LINA_ASSERT(it != m_worldRenderers.end(), "");
		WorldRenderer* wr = it->second;
		m_gfxManager->RemoveRenderer(wr);
		delete wr;
		m_worldRenderers.erase(it);
	}

	WorldRenderer* Editor::GetWorldRenderer(EntityWorld* world)
	{
		return m_worldRenderers.at(world);
	}

	void Editor::OnResourceLoadEnded(int32 taskID, const Vector<ResourceIdentifier>& idents)
	{
		if (taskID == RTID_CORE_RES)
		{
			CoreResourcesLoaded();
		}
	}

} // namespace Lina::Editor
