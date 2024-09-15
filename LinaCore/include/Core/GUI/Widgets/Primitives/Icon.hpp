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

#include "Core/GUI/Widgets/Widget.hpp"
#include "Common/Data/String.hpp"

#include "Common/Platform/LinaVGIncl.hpp"

namespace Lina
{
	class Font;

	class Icon : public Widget
	{
	public:
		Icon() : Widget(WF_OWNS_SIZE)
		{
		}
		virtual ~Icon() = default;

		struct Properties
		{
			bool			 _fold = false;
			Delegate<void()> onClicked;
			String			 icon					= "";
			ResourceID		 font					= Theme::GetDef().iconFont;
			ColorGrad		 color					= Theme::GetDef().foreground0;
			Color			 colorHovered			= Theme::GetDef().foreground0;
			Color			 colorPressed			= Theme::GetDef().foreground0;
			Color			 colorDisabled			= Theme::GetDef().silent1;
			float			 textScale				= 0.5f;
			bool			 isDynamic				= true;
			bool			 enableHoverPressColors = false;

			Color sdfOutlineColor	  = Color::White;
			float sdfThickness		  = 0.5f;
			float sdfSoftness		  = 0.024f;
			float sdfOutlineThickness = 0.0f;
			float sdfOutlineSoftness  = 0.0f;

			Vector4 customClip			= Vector4::Zero;
			bool	dynamicSizeToParent = false;
			float	dynamicSizeScale	= 1.0f;

			void SaveToStream(OStream& stream) const
			{
				stream << color << colorHovered << colorPressed << colorDisabled << sdfOutlineColor << customClip;
				stream << icon;
				stream << font << textScale << isDynamic << enableHoverPressColors << sdfThickness << sdfSoftness << sdfOutlineThickness << sdfOutlineSoftness;
				stream << dynamicSizeToParent << dynamicSizeScale;
			}

			void LoadFromStream(IStream& stream)
			{
				stream >> color >> colorHovered >> colorPressed >> colorDisabled >> sdfOutlineColor >> customClip;
				stream >> icon;
				stream >> font >> textScale >> isDynamic >> enableHoverPressColors >> sdfThickness >> sdfSoftness >> sdfOutlineThickness >> sdfOutlineSoftness;
				stream >> dynamicSizeToParent >> dynamicSizeScale;
			}
		};

		virtual void Initialize() override;
		virtual void CalculateSize(float dt) override;
		virtual void Draw() override;
		virtual bool OnMouse(uint32 button, LinaGX::InputAction act) override;

		void CalculateIconSize();

		inline Properties& GetProps()
		{
			return m_props;
		}

		inline LinaVG::LinaVGFont* GetLVGFont()
		{
			return m_lvgFont;
		}

		inline virtual void SaveToStream(OStream& stream) const override
		{
			Widget::SaveToStream(stream);
			m_props.SaveToStream(stream);
		}

		inline virtual void LoadFromStream(IStream& stream) override
		{
			Widget::LoadFromStream(stream);
			m_props.LoadFromStream(stream);
		}

	private:
		LINA_REFLECTION_ACCESS(Icon);

		Properties			   m_props				= {};
		LinaVG::SDFTextOptions m_sdfOptions			= {};
		float				   m_calculatedDPIScale = 0.0f;
		LinaVG::LinaVGFont*	   m_lvgFont			= nullptr;
	};

	LINA_WIDGET_BEGIN(Icon, Primitive)
	LINA_FIELD(Icon, m_props, "", "Class", GetTypeID<Icon::Properties>());
	LINA_CLASS_END(Icon)

	LINA_CLASS_BEGIN(IconProperties)
	LINA_FIELD(Icon::Properties, _fold, "Icon", "Category", 0)
	LINA_FIELD(Icon::Properties, icon, "Icon", "String", 0)
	LINA_FIELD(Icon::Properties, font, "Font", "Font", 0)
	LINA_FIELD(Icon::Properties, textScale, "Scale", "float", 0)
	LINA_FIELD(Icon::Properties, isDynamic, "Dynamic", "bool", 0)
	LINA_FIELD(Icon::Properties, enableHoverPressColors, "Enable Hover/Press Colors", "bool", 0)
	LINA_FIELD(Icon::Properties, colorHovered, "Color Hovered", "Color", 0)
	LINA_FIELD(Icon::Properties, colorPressed, "Color Pressed", "Color", 0)
	LINA_FIELD(Icon::Properties, color, "Color", "ColorGrad", 0)
	LINA_FIELD(Icon::Properties, colorDisabled, "Color Disabled", "Color", 0)
	LINA_FIELD(Icon::Properties, customClip, "Custom Clip", "Rect", 0)
	LINA_FIELD(Icon::Properties, dynamicSizeToParent, "Dynamic Size To Parent", "bool", 0)
	LINA_FIELD(Icon::Properties, dynamicSizeScale, "Scale", "float", 0)
	LINA_FIELD(Icon::Properties, sdfOutlineColor, "SDF Outline Color", "Color", 0)
	LINA_FIELD(Icon::Properties, sdfThickness, "SDF Thickness", "float", 0)
	LINA_FIELD(Icon::Properties, sdfSoftness, "SDF Softness", "float", 0)
	LINA_FIELD(Icon::Properties, sdfOutlineThickness, "SDF Outline Thickness", "float", 0)
	LINA_FIELD(Icon::Properties, sdfOutlineSoftness, "SDF Outline Softness", "float", 0)

	LINA_FIELD_DEPENDENCY_POS(Icon::Properties, colorHovered, "enableHoverPressColors", 1)
	LINA_FIELD_DEPENDENCY_POS(Icon::Properties, colorPressed, "enableHoverPressColors", 1)
	LINA_FIELD_DEPENDENCY_POS(Icon::Properties, dynamicSizeScale, "dynamicSizeToParent", 1)
	LINA_CLASS_END(IconProperties)

} // namespace Lina
