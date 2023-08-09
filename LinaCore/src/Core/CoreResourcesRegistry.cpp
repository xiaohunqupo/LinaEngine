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

#include "Core/CoreResourcesRegistry.hpp"
#include "Resources/Core/ResourceManager.hpp"
#include "Graphics/Resource/Model.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Graphics/Resource/TextureSampler.hpp"
#include "Graphics/Resource/Font.hpp"
#include "Graphics/Resource/Shader.hpp"
#include "Graphics/Resource/Material.hpp"
#include "Graphics/Core/CommonGraphics.hpp"
#include "World/Level/Level.hpp"

namespace Lina
{
	void CoreResourcesRegistry::RegisterResourceTypes(ResourceManager& rm)
	{
		rm.RegisterResourceType<Model>(50, {"glb"}, PackageType::Package1);
		rm.RegisterResourceType<Shader>(25, {"linashader"}, PackageType::Package1);
		rm.RegisterResourceType<Texture>(100, {"png", "jpeg", "jpg"}, PackageType::Package1);
		rm.RegisterResourceType<TextureSampler>(100, {"linasampler"}, PackageType::Package1);
		rm.RegisterResourceType<Font>(10, {"ttf", "otf"}, PackageType::Package1);
		rm.RegisterResourceType<Material>(25, {"linamaterial"}, PackageType::Package1);
		rm.RegisterResourceType<Level>(10, {"linalevel"}, PackageType::PackageLevels);
	}

	Vector<ResourceIdentifier> CoreResourcesRegistry::GetPriorityResources()
	{
		Vector<ResourceIdentifier> list;

		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_1x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_2x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_3x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_4x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Textures/StubBlack.png", GetTypeID<Texture>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Shaders/LitStandard.linashader", GetTypeID<Shader>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Shaders/UnlitStandard.linashader", GetTypeID<Shader>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Shaders/GUIStandard.linashader", GetTypeID<Shader>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Shaders/ScreenQuads/SQTexture.linashader", GetTypeID<Shader>(), 0));

		for (auto& ident : list)
			ident.sid = TO_SID(ident.path);

		return list;
	}

	Vector<Pair<StringID, ResourceMetadata>> CoreResourcesRegistry::GetPriorityResourcesMetadata()
	{
		Vector<Pair<StringID, ResourceMetadata>> list;
		ResourceMetadata						 metadata;

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 12);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_1x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 14);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_2x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 16);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_3x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 20);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_4x.ttf"_hs, metadata));
		metadata.ClearAll();

		return list;
	}

	Vector<ResourceIdentifier> CoreResourcesRegistry::GetCoreResources()
	{
		Vector<ResourceIdentifier> list;

		list.push_back(ResourceIdentifier("Resources/Core/Textures/StubLinaLogo.png", GetTypeID<Texture>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Textures/StubLinaLogoWhite.png", GetTypeID<Texture>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Textures/StubLinaLogoText.png", GetTypeID<Texture>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/WorkSans-Regular_1x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/WorkSans-Regular_2x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/WorkSans-Regular_3x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/WorkSans-Regular_4x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_Bold_1x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_Bold_2x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_Bold_3x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/NunitoSans_Bold_4x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/Rubik-Regular_1x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/Rubik-Regular_2x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/Rubik-Regular_3x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Fonts/Rubik-Regular_4x.ttf", GetTypeID<Font>(), 0));
		list.push_back(ResourceIdentifier("Resources/Core/Models/LinaLogo.glb", GetTypeID<Model>(), 0));

		for (auto& ident : list)
			ident.sid = TO_SID(ident.path);

		return list;
	}

	Vector<Pair<StringID, ResourceMetadata>> CoreResourcesRegistry::GetCoreResourcesMetadata()
	{
		Vector<Pair<StringID, ResourceMetadata>> list;
		ResourceMetadata						 metadata;

		//-------------
		metadata.SetSID(TEXTURE_META_SAMPLER_SID, DEFAULT_GUI_SAMPLER_SID);
		list.push_back(linatl::make_pair("Resources/Core/Textures/StubLinaLogo.png"_hs, metadata));
		list.push_back(linatl::make_pair("Resources/Core/Textures/StubLinaLogoWhite.png"_hs, metadata));
		list.push_back(linatl::make_pair("Resources/Core/Textures/StubLinaLogoText.png"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 13);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/WorkSans-Regular_1x.ttf"_hs, metadata));
		metadata.ClearAll();

		////-------------
		metadata.SetInt(FONT_META_SIZE, 15);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/WorkSans-Regular_2x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 18);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/WorkSans-Regular_3x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 22);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/WorkSans-Regular_4x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 12);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_Bold_1x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 14);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_Bold_2x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 16);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_Bold_3x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetBool(FONT_META_ISSDF, false);
		metadata.SetInt(FONT_META_SIZE, 20);
		metadata.SetInt(FONT_META_CUSTOM_GLPYHS, 2);
		metadata.SetInt("Range_0"_hs, 160);
		metadata.SetInt("Range_1"_hs, 380);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/NunitoSans_Bold_4x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 12);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/Rubik-Regular_1x.ttf"_hs, metadata));
		metadata.ClearAll();

		////-------------
		metadata.SetInt(FONT_META_SIZE, 14);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/Rubik-Regular_2x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 16);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/Rubik-Regular_3x.ttf"_hs, metadata));
		metadata.ClearAll();

		//-------------
		metadata.SetInt(FONT_META_SIZE, 20);
		metadata.SetBool(FONT_META_ISSDF, false);
		list.push_back(linatl::make_pair("Resources/Core/Fonts/Rubik-Regular_4x.ttf"_hs, metadata));
		metadata.ClearAll();

		return list;
	}

} // namespace Lina
