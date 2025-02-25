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

#include "Editor/Widgets/Panel/PanelShaderViewer.hpp"
#include "Editor/Editor.hpp"
#include "Editor/Widgets/World/WorldDisplayer.hpp"
#include "Core/GUI/Widgets/WidgetManager.hpp"
#include "Core/GUI/Widgets/Layout/FoldLayout.hpp"
#include "Core/GUI/Widgets/Primitives/InputField.hpp"
#include "Core/Graphics/Resource/Texture.hpp"

namespace Lina::Editor
{

	void PanelShaderViewer::Construct()
	{
		PanelResourceViewer::Construct();

		WorldDisplayer* displayer = m_manager->Allocate<WorldDisplayer>("WorldDisplayer");
		displayer->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
		displayer->SetAlignedPos(Vector2::Zero);
		displayer->SetAlignedSize(Vector2::One);
		m_resourceBG->AddChild(displayer);
		m_worldDisplayer = displayer;
	}

	void PanelShaderViewer::Initialize()
	{
		PanelResourceViewer::Initialize();
		if (!m_resource)
			return;

		if (m_world == nullptr && m_worldRenderer == nullptr)
		{
		}
	}

	void PanelShaderViewer::Destruct()
	{
		PanelResourceViewer::Destruct();

		// m_editor->GetEditorRenderer().RemoveWorldRenderer(m_worldRenderer);
		// delete m_worldRenderer;
		// delete m_world;
	}

	void PanelShaderViewer::PreTick()
	{
	}

	void PanelShaderViewer::Tick(float delta)
	{
	}
	/*
		void PanelShaderViewer::Construct()
		{
			m_editor = Editor::Get();

			DirectionalLayout* horizontal = m_manager->Allocate<DirectionalLayout>("Horizontal");
			horizontal->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			horizontal->SetAlignedPos(Vector2::Zero);
			horizontal->SetAlignedSize(Vector2::One);
			horizontal->GetProps().direction = DirectionOrientation::Horizontal;
			horizontal->GetProps().mode		 = DirectionalLayout::Mode::Bordered;
			AddChild(horizontal);

			Widget* worldBG = m_manager->Allocate<Widget>("ShaderBG");
			worldBG->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			worldBG->SetAlignedPos(Vector2(0.0f, 0.0f));
			worldBG->SetAlignedSize(Vector2(0.75f, 1.0f));
			worldBG->GetWidgetProps().childMargins	  = TBLR::Eq(Theme::GetDef().baseIndent);
			worldBG->GetWidgetProps().drawBackground  = true;
			worldBG->GetWidgetProps().colorBackground = Theme::GetDef().background0;
			worldBG->GetWidgetProps().childMargins	  = TBLR::Eq(Theme::GetDef().baseIndent);
			horizontal->AddChild(worldBG);

			WorldDisplayer* displayer = m_manager->Allocate<WorldDisplayer>("WorldDisplayer");
			displayer->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			displayer->SetAlignedPos(Vector2::Zero);
			displayer->SetAlignedSize(Vector2::One);
			worldBG->AddChild(displayer);

			DirectionalLayout* right = m_manager->Allocate<DirectionalLayout>("RightSide");
			right->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			right->SetAlignedPos(Vector2(0.75f, 0.0f));
			right->SetAlignedSize(Vector2(0.25f, 1.0f));
			right->GetProps().direction = DirectionOrientation::Vertical;
			right->GetProps().mode		= DirectionalLayout::Mode::Bordered;
			horizontal->AddChild(right);

			Widget* inspectorParent = m_manager->Allocate<Widget>("InspectorParent");
			inspectorParent->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			inspectorParent->SetAlignedPos(Vector2(0.0f, 0.0f));
			inspectorParent->SetAlignedSize(Vector2(1.0f, 0.5f));
			right->AddChild(inspectorParent);

			Widget* hierarchyParent = m_manager->Allocate<Widget>("HierarchyParent");
			hierarchyParent->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			hierarchyParent->SetAlignedPos(Vector2(0.0f, 0.5f));
			hierarchyParent->SetAlignedSize(Vector2(1.0f, 0.5f));
			right->AddChild(hierarchyParent);

			ScrollArea* scrollInspector = m_manager->Allocate<ScrollArea>("Scroll");
			scrollInspector->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			scrollInspector->SetAlignedPos(Vector2::Zero);
			scrollInspector->SetAlignedSize(Vector2::One);
			scrollInspector->GetProps().direction = DirectionOrientation::Vertical;
			inspectorParent->AddChild(scrollInspector);

			DirectionalLayout* inspector = m_manager->Allocate<DirectionalLayout>("Inspector");
			inspector->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y | WF_SIZE_ALIGN_X | WF_SIZE_ALIGN_Y);
			inspector->SetAlignedPos(Vector2::Zero);
			inspector->SetAlignedSize(Vector2::One);
			inspector->GetProps().direction				  = DirectionOrientation::Vertical;
			inspector->GetWidgetProps().childPadding	  = Theme::GetDef().baseIndentInner;
			inspector->GetWidgetProps().clipChildren	  = true;
			inspector->GetWidgetProps().childMargins.left = Theme::GetDef().baseBorderThickness;
			inspector->GetWidgetProps().childMargins.top  = Theme::GetDef().baseIndent;
			scrollInspector->AddChild(inspector);

			m_worldDisplayer = displayer;
			m_inspector		 = inspector;
		}

		void PanelShaderViewer::Initialize()
		{
			if (m_shader != nullptr)
				return;

			if (m_editor->GetProjectManager().GetProjectData() == nullptr)
				return;

			if (!m_editor->GetProjectManager().GetProjectData()->GetResourceRoot().FindResource(m_subData))
			{
				Text* text = m_manager->Allocate<Text>("Info");
				text->GetFlags().Set(WF_POS_ALIGN_X | WF_POS_ALIGN_Y);
				text->SetAlignedPos(Vector2(0.5f, 0.5f));
				text->SetAnchorX(Anchor::Center);
				text->SetAnchorY(Anchor::Center);
				text->GetProps().text = Locale::GetStr(LocaleStr::ThisResourceNoLongerExists);
				m_worldDisplayer->AddChild(text);
				m_worldDisplayer->DisplayWorld(nullptr);
				return;
			}

			ResourceDef def = {
				.id	 = m_subData,
				.tid = GetTypeID<Shader>(),
			};
			m_editor->GetApp()->GetResourceManager().LoadResourcesFromProject(m_editor, m_editor->GetProjectManager().GetProjectData(), {def}, 0);
			m_editor->GetApp()->GetResourceManager().WaitForAll();
			m_shader				   = m_editor->GetApp()->GetResourceManager().GetResource<Shader>(def.id);
			m_shaderName			   = m_shader->GetName();
			GetWidgetProps().debugName = "Shader: " + m_shader->GetName();

			FoldLayout* foldGeneral = CommonWidgets::BuildFoldTitle(this, Locale::GetStr(LocaleStr::General), &m_generalFold);
			FoldLayout* foldShader	= CommonWidgets::BuildFoldTitle(this, "Shader", &m_shaderFold);
			m_inspector->AddChild(foldGeneral);
			m_inspector->AddChild(foldShader);

			CommonWidgets::BuildClassReflection(foldGeneral, this, ReflectionSystem::Get().Resolve<PanelShaderViewer>(), [this](MetaType* meta, FieldBase* field) {

			});

			CommonWidgets::BuildClassReflection(foldShader, &m_shader->GetMeta(), ReflectionSystem::Get().Resolve<Shader::Metadata>(), [this](MetaType* meta, FieldBase* field) {
				SetRuntimeDirty(true);
				// RegenShader("");
			});

			auto buildButtonLayout = [this]() -> Widget* {
				DirectionalLayout* layout = m_manager->Allocate<DirectionalLayout>();
				layout->GetFlags().Set(WF_POS_ALIGN_X | WF_SIZE_ALIGN_X | WF_USE_FIXED_SIZE_Y);
				layout->SetAlignedPosX(0.0f);
				layout->SetFixedSizeY(Theme::GetDef().baseItemHeight);
				layout->SetAlignedSizeX(1.0f);
				layout->GetProps().mode						= DirectionalLayout::Mode::EqualSizes;
				layout->GetProps().direction				= DirectionOrientation::Horizontal;
				layout->GetWidgetProps().childMargins.left	= Theme::GetDef().baseIndent;
				layout->GetWidgetProps().childMargins.right = Theme::GetDef().baseIndent;
				layout->GetWidgetProps().childPadding		= Theme::GetDef().baseIndent;
				return layout;
			};

			Widget* buttonLayout1 = buildButtonLayout();
			Widget* buttonLayout2 = buildButtonLayout();
			foldGeneral->AddChild(buttonLayout1);
			foldGeneral->AddChild(buttonLayout2);
			static_cast<DirectionalLayout*>(buttonLayout1)->GetProps().mode = DirectionalLayout::Mode::EqualSizes;
			static_cast<DirectionalLayout*>(buttonLayout2)->GetProps().mode = DirectionalLayout::Mode::EqualSizes;

			Button* importButton = WidgetUtility::BuildIconTextButton(this, ICON_IMPORT, Locale::GetStr(LocaleStr::Import));
			importButton->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
			importButton->SetAlignedPosY(0.0f);
			importButton->SetAlignedSizeY(1.0f);
			importButton->SetFixedSizeX(Theme::GetDef().baseItemHeight * 4);
			importButton->GetProps().onClicked = [this]() {
				const Vector<String> paths = PlatformProcess::OpenDialog({
					.title				   = Locale::GetStr(LocaleStr::Import),
					.primaryButton		   = Locale::GetStr(LocaleStr::Import),
					.extensionsDescription = "",
					.extensions			   = {"linashader"},
					.mode				   = PlatformProcess::DialogMode::SelectFile,
				});
				if (paths.empty())
					return;

				RegenShader(paths.front());
				SetRuntimeDirty(true);
			};
			buttonLayout1->AddChild(importButton);

			Button* reimportButton = WidgetUtility::BuildIconTextButton(this, ICON_ROTATE, Locale::GetStr(LocaleStr::ReImport));
			reimportButton->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
			reimportButton->SetAlignedPosY(0.0f);
			reimportButton->SetAlignedSizeY(1.0f);
			reimportButton->SetFixedSizeX(Theme::GetDef().baseItemHeight * 4);
			reimportButton->GetProps().onClicked = [this, reimportButton]() {
				const String path = m_shader->GetPath();
				if (!FileSystem::FileOrPathExists(path))
				{
					CommonWidgets::ThrowInfoTooltip(Locale::GetStr(LocaleStr::FileNotFound), LogLevel::Error, 3.0f, reimportButton);
					return;
				}

				RegenShader(path);
				SetRuntimeDirty(true);
			};

			buttonLayout1->AddChild(reimportButton);

			Button* save = WidgetUtility::BuildIconTextButton(this, ICON_SAVE, Locale::GetStr(LocaleStr::Save));
			save->GetFlags().Set(WF_POS_ALIGN_Y | WF_SIZE_ALIGN_Y);
			save->SetAlignedPosY(0.0f);
			save->SetAlignedSizeY(1.0f);
			save->SetFixedSizeX(Theme::GetDef().baseItemHeight * 4);
			save->GetProps().onClicked = [this]() {
				if (m_containsRuntimeChanges)
				{
					m_resourceManager->SaveResource(m_editor->GetProjectManager().GetProjectData(), m_shader);
					// m_editor->GetResourcePipeline().GenerateThumbnailForResource(m_editor->GetProjectManager().GetProjectData()->GetResourceRoot().FindResource(m_shader->GetID()), m_shader, true);
					Panel* p = m_editor->GetWindowPanelManager().FindPanelOfType(PanelType::ResourceBrowser, 0);
					if (p)
						static_cast<PanelResourceBrowser*>(p)->GetBrowser()->RefreshDirectory();
					SetRuntimeDirty(false);
				}
			};

			m_saveButton = save;
			buttonLayout2->AddChild(save);

			SetupScene();

			// Add rendering
			m_editor->GetEditorRenderer().AddWorldRenderer(m_worldRenderer);
			m_worldDisplayer->DisplayWorld(m_worldRenderer);
			Panel::Initialize();
		}

		void PanelShaderViewer::SetupScene()
		{

			// Setup world
			m_world            = new EntityWorld(0, "");
			m_lastWorldSize = Vector2ui(4, 4);
			m_worldRenderer = new WorldRenderer(m_world, m_lastWorldSize);

			Vector<ResourceDef> neededResources = {
				{
					.id      = DEFAULT_SHADER_LIGHTING_ID,
					.name = DEFAULT_SHADER_LIGHTING_PATH,
					.tid  = GetTypeID<Shader>(),
				},
				{
					.id      = DEFAULT_SHADER_SKY_ID,
					.name = DEFAULT_SHADER_SKY_PATH,
					.tid  = GetTypeID<Shader>(),
				},
				{
					.id      = DEFAULT_SHADER_OBJECT_ID,
					.name = DEFAULT_SHADER_OBJECT_PATH,
					.tid  = GetTypeID<Shader>(),
				},
				{
					.id      = DEFAULT_SKY_CUBE_ID,
					.name = DEFAULT_SKY_CUBE_PATH,
					.tid  = GetTypeID<Model>(),
				},
				{
					.id      = DEFAULT_MODEL_CUBE_ID,
					.name = DEFAULT_MODEL_CUBE_PATH,
					.tid  = GetTypeID<Model>(),
				},
				{
					.id      = DEFAULT_MODEL_SPHERE_ID,
					.name = DEFAULT_MODEL_SPHERE_PATH,
					.tid  = GetTypeID<Model>(),
				},
				{
					.id      = DEFAULT_MODEL_PLANE_ID,
					.name = DEFAULT_MODEL_PLANE_PATH,
					.tid  = GetTypeID<Model>(),
				},
				{
					.id      = DEFAULT_MODEL_CYLINDER_ID,
					.name = DEFAULT_MODEL_CYLINDER_PATH,
					.tid  = GetTypeID<Model>(),
				},
			};

			m_world->GetResourceManagerV2().LoadResourcesFromFile(m_editor, neededResources, -1);

			neededResources.clear();
			neededResources.push_back({.id = m_shader->GetID(), .name = m_shader->GetName(), .tid = GetTypeID<Shader>()});

			m_world->GetResourceManagerV2().LoadResourcesFromProject(m_editor, m_editor->GetProjectManager().GetProjectData(), neededResources, -1);
			m_world->GetResourceManagerV2().WaitForAll();

			Material* skyMaterial	   = m_world->GetResourceManagerV2().CreateResource<Material>(m_world->GetResourceManagerV2().ConsumeResourceID(), "Shader Viewer Sky Material");
			Material* lightingMaterial = m_world->GetResourceManagerV2().CreateResource<Material>(m_world->GetResourceManagerV2().ConsumeResourceID(), "Shader Viewer Lighting Material");
			Shader*	  lightingShader   = m_world->GetResourceManagerV2().GetResource<Shader>(DEFAULT_SHADER_LIGHTING_ID);
			Shader*	  skyShader		   = m_world->GetResourceManagerV2().GetResource<Shader>(DEFAULT_SHADER_SKY_ID);
			lightingMaterial->SetShader(lightingShader);
			skyMaterial->SetShader(skyShader);
			skyMaterial->SetProperty("topColor"_hs, Color(0.8f, 0.8f, 0.8f, 1.0f));
			skyMaterial->SetProperty("groundColor"_hs, Color(0.1f, 0.1f, 0.1f, 1.0f));
			m_world->GetGfxSettings().SetSkyMaterial(skyMaterial);
			m_world->GetGfxSettings().SetLightingMaterial(lightingMaterial);

			// Add Shader to world.

			Model* cube = m_world->GetResourceManagerV2().GetResource<Model>(DEFAULT_MODEL_CUBE_ID);
			Material* shaderMaterial = m_world->GetResourceManagerV2().CreateResource<Material>(m_world->GetResourceManagerV2().ConsumeResourceID(), "Shader Material");
			shaderMaterial->SetShader(m_world->GetResourceManagerV2().GetResource<Shader>(m_shader->GetID()));
			Entity* entityCube = WorldUtility::AddModelToWorld(m_world,cube, {shaderMaterial});

			Entity* camera = m_world->CreateEntity("Camera");
			CameraComponent*   cameraComp = m_world->AddComponent<CameraComponent>(camera);
			FlyCameraMovement* flight      = m_world->AddComponent<FlyCameraMovement>(camera);

			const Vector3 cameraPosition = Vector3(0, 2.0f, 4.0f);
			camera->SetPosition(cameraPosition);
			camera->SetRotation(Quaternion::LookAt(cameraPosition, Vector3::Zero, Vector3::Up));
			flight->GetFlags()              = CF_RECEIVE_EDITOR_TICK;
			m_world->SetActiveCamera(cameraComp);

		}

		void PanelShaderViewer::Destruct()
		{
			Panel::Destruct();
			if (m_shader == nullptr)
				return;

			m_editor->GetApp()->GetResourceManager().UnloadResources({m_shader});
			m_editor->GetEditorRenderer().RemoveWorldRenderer(m_worldRenderer);
			delete m_worldRenderer;
			delete m_world;
		}

		void PanelShaderViewer::PreTick()
		{
			if (m_world == nullptr)
				return;

			const Vector2ui sz = m_worldDisplayer->GetSize();

			if (sz.x != 0 && sz.y != 0 && !sz.Equals(m_lastWorldSize))
			{
				m_lastWorldSize = sz;
				Application::GetLGX()->Join();
				m_worldRenderer->Resize(m_lastWorldSize);
				m_editor->GetEditorRenderer().RefreshDynamicTextures();
			}

			m_world->PreTick(ComponentFlags::CF_RECEIVE_EDITOR_TICK);
		}

		void PanelShaderViewer::Tick(float delta)
		{
			if (m_world == nullptr)
				return;

			m_world->Tick(delta, ComponentFlags::CF_RECEIVE_EDITOR_TICK);
		}

		void PanelShaderViewer::SetRuntimeDirty(bool isDirty)
		{
			m_containsRuntimeChanges = isDirty;
			Text* txt				 = GetWidgetOfType<Text>(m_saveButton);
			if (isDirty)
				txt->GetProps().text = Locale::GetStr(LocaleStr::Save) + " *";
			else
				txt->GetProps().text = Locale::GetStr(LocaleStr::Save);
			txt->CalculateTextSize();
		}

		void PanelShaderViewer::RegenShader(const String& path)
		{
			Application::GetLGX()->Join();
			m_font->DestroyHW();

			if (!path.empty())
				m_font->LoadFromFile(path);

			m_font->GenerateHW(m_editor->GetEditorRenderer().GetGUIBackend().GetLVGText());
			m_editor->GetEditorRenderer().MarkBindlessDirty();
			m_fontDisplay->GetProps().font = m_font->GetID();
			m_fontDisplay->CalculateTextSize();
		}

		void PanelShaderViewer::SaveLayoutToStream(OStream& stream)
		{
		}

		void PanelShaderViewer::LoadLayoutFromStream(IStream& stream)
		{
		}

	*/
} // namespace Lina::Editor
