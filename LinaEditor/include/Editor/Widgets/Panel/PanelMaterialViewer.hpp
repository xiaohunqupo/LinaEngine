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

#include "Editor/Widgets/Panel/PanelResourceViewer.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Core/Graphics/CommonGraphics.hpp"

namespace Lina
{
	class EntityWorld;
	class WorldRenderer;
	class Entity;
} // namespace Lina

namespace Lina::Editor
{
	class Editor;
	class WorldDisplayer;
	class EditorWorldRenderer;

	enum class MaterialViewerDisplayType
	{
		Cube,
		Sphere,
		Plane,
		Capsule,
		Cylinder,
		Skybox,
	};

	class PanelMaterialViewer : public PanelResourceViewer
	{
	public:
		PanelMaterialViewer() : PanelResourceViewer(PanelType::MaterialViewer, GetTypeID<Material>(), GetTypeID<PanelMaterialViewer>()){};
		virtual ~PanelMaterialViewer() = default;

		virtual void Construct() override;
		virtual void Destruct() override;
		virtual void Initialize() override;
		virtual void StoreEditorActionBuffer() override;
		virtual void UpdateResourceProperties() override;
		virtual void RebuildContents() override;

		void StoreShaderID();
		void SetupWorld();

		inline EntityWorld* GetWorld() const
		{
			return m_world;
		}

	private:
		void UpdateMaterial();

	private:
		LINA_REFLECTION_ACCESS(PanelMaterialViewer);

		Entity*					  m_displayEntity  = nullptr;
		MaterialViewerDisplayType m_displayType	   = MaterialViewerDisplayType::Cube;
		String					  m_materialName   = "";
		WorldDisplayer*			  m_worldDisplayer = nullptr;
		EntityWorld*			  m_world		   = nullptr;
		ShaderType				  m_shaderType	   = ShaderType::DeferredSurface;
		String					  m_shaderTypeStr  = "";
		Vector<uint32>			  m_propertyFoldValues;
		OStream					  m_previousStream;
		ResourceID				  m_shaderID			  = 0;
		ResourceID				  m_previousShaderID	  = 0;
		uint8*					  m_latestColorWheelData  = nullptr;
		uint32					  m_latestColorWheelComps = 0;
		Material*				  m_defaultSky			  = nullptr;
	};

	LINA_CLASS_BEGIN(MaterialViewerDisplayType)
	LINA_PROPERTY_STRING(MaterialViewerDisplayType, 0, "Cube")
	LINA_PROPERTY_STRING(MaterialViewerDisplayType, 1, "Sphere")
	LINA_PROPERTY_STRING(MaterialViewerDisplayType, 2, "Plane")
	LINA_PROPERTY_STRING(MaterialViewerDisplayType, 3, "Capsule")
	LINA_PROPERTY_STRING(MaterialViewerDisplayType, 4, "Cylinder")
	LINA_CLASS_END(MaterialViewerDisplayType)

	LINA_WIDGET_BEGIN(PanelMaterialViewer, Hidden)
	LINA_FIELD(PanelMaterialViewer, m_materialName, "Name", FieldType::StringFixed, 0)
	LINA_FIELD(PanelMaterialViewer, m_shaderTypeStr, "Shader Type", FieldType::StringFixed, 0)
	LINA_FIELD(PanelMaterialViewer, m_displayType, "Display Type", FieldType::Enum, GetTypeID<MaterialViewerDisplayType>())
	LINA_CLASS_END(PanelMaterialViewer)

} // namespace Lina::Editor
