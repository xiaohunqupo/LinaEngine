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
#include "Core/Audio/Audio.hpp"
#include "Core/Graphics/CommonGraphics.hpp"

namespace Lina
{

} // namespace Lina

namespace Lina::Editor
{
	class Editor;
	class WorldDisplayer;

	class PanelAudioViewer : public PanelResourceViewer
	{
	public:
		PanelAudioViewer() : PanelResourceViewer(PanelType::AudioViewer, GetTypeID<Audio>(), GetTypeID<PanelAudioViewer>()){};
		virtual ~PanelAudioViewer() = default;

		virtual void Construct() override;
		virtual void Initialize() override;

	private:
		void UpdateAudioProps();

	private:
		LINA_REFLECTION_ACCESS(PanelAudioViewer);

		String m_audioName = "";
	};

	LINA_WIDGET_BEGIN(PanelAudioViewer, Hidden)
	LINA_FIELD(PanelAudioViewer, m_audioName, "Name", FieldType::StringFixed, 0)
	LINA_CLASS_END(PanelAudioViewer)

} // namespace Lina::Editor
