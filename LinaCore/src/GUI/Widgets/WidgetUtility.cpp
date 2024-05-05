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

#include "Core/GUI/Widgets/WidgetUtility.hpp"
#include "Core/GUI/Widgets/Layout/DirectionalLayout.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/InputField.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Common/Math/Rect.hpp"
#include "Common/Math/Math.hpp"

namespace Lina
{

	void WidgetUtility::DrawRectBackground(LinaVG::Drawer* lvg, const RectBackground& options, const Rect& rect, int32 drawOrder, bool positionIsCenter)
	{
		if (!options.enabled)
			return;

		LinaVG::StyleOptions opts;
		opts.color.start		 = options.startColor.AsLVG4();
		opts.color.end			 = options.endColor.AsLVG4();
		opts.rounding			 = options.rounding;
		const LinaVG::Vec2 start = positionIsCenter ? (rect.pos - rect.size * 0.5f).AsLVG() : (rect.pos).AsLVG();
		const LinaVG::Vec2 end	 = positionIsCenter ? (rect.pos + rect.size * 0.5f).AsLVG() : (rect.pos + rect.size).AsLVG();
		lvg->DrawRect(start, end, opts, 0.0f, drawOrder);
	}

	void WidgetUtility::DrawAlphaLine(LinaVG::Drawer* lvg, const Vector2& start, const Vector2& end, int32 drawOrder, const Color& baseColor)
	{
		Color weak = baseColor, strong = baseColor;
		strong.w = 1.0f;
		weak.w	 = 0.0f;

		LinaVG::StyleOptions style;
		style.color.start = strong.AsLVG4();
		style.color.end	  = weak.AsLVG4();
		lvg->DrawRect(start.AsLVG(), end.AsLVG(), style, 0.0f, drawOrder);
	}

	void WidgetUtility::DrawAlphaLineCentral(LinaVG::Drawer* lvg, const Vector2& start, const Vector2& end, int32 drawOrder, const Color& baseColor)
	{
		Color weak = baseColor, strong = baseColor;
		strong.w = 1.0f;
		weak.w	 = 0.0f;

		LinaVG::StyleOptions style;
		style.color.start = weak.AsLVG4();
		style.color.end	  = strong.AsLVG4();

		LinaVG::StyleOptions style2;
		style2.color.start = strong.AsLVG4();
		style2.color.end   = weak.AsLVG4();

		lvg->DrawRect(start.AsLVG(), Vector2((end.x + start.x) * 0.5f, end.y).AsLVG(), style, 0.0f, drawOrder);
		lvg->DrawRect(Vector2((end.x + start.x) * 0.5f, start.y).AsLVG(), Vector2(end.x, end.y).AsLVG(), style2, 0.0f, drawOrder);
	}

	void WidgetUtility::DrawGradLineCentral(LinaVG::Drawer* lvg, const Vector2& start, const Vector2& end, int32 drawOrder, const Color& centerColor, const Color& edgeColor)
	{
		Color weak = edgeColor, strong = centerColor;

		LinaVG::StyleOptions style;
		style.color.start = weak.AsLVG4();
		style.color.end	  = strong.AsLVG4();

		LinaVG::StyleOptions style2;
		style2.color.start = strong.AsLVG4();
		style2.color.end   = weak.AsLVG4();

		lvg->DrawRect(start.AsLVG(), Vector2((end.x + start.x) * 0.5f, end.y).AsLVG(), style, 0.0f, drawOrder);
		lvg->DrawRect(Vector2((end.x + start.x) * 0.5f, start.y).AsLVG(), Vector2(end.x, end.y).AsLVG(), style2, 0.0f, drawOrder);
	}

	void WidgetUtility::DrawDropShadow(LinaVG::Drawer* lvg, const Vector2& p1, const Vector2& p2, int32 drawOrder, const Color& baseColor, int32 radius)
	{
		const Color endColor = Color(baseColor.x, baseColor.y, baseColor.z, 0.0f);

		Vector2 startPos = p1;
		Vector2 endPos	 = p2;

		const Vector2 lineDir = (endPos - startPos).Normalized().Rotate(90.0f);

		for (int32 i = 0; i < radius; i++)
		{
			const Color			 color = Math::Lerp(baseColor, endColor, static_cast<float>(i) / static_cast<float>(radius));
			LinaVG::StyleOptions style;
			style.color = color.AsLVG4();
			lvg->DrawLine(startPos.AsLVG(), endPos.AsLVG(), style, LinaVG::LineCapDirection::None, 0.0f, drawOrder);
			startPos += lineDir;
			endPos += lineDir;
		}
	}

	void WidgetUtility::DrawDropShadowRect(LinaVG::Drawer* lvg, const Rect& rect, int32 drawOrder, const Color& baseColor, int32 radius)
	{
		Rect usedRect = rect;

		const Color endColor = Color(baseColor.x, baseColor.y, baseColor.z, 0.0f);

		for (int32 i = 0; i < radius; i++)
		{
			const Color			 color = Math::Lerp(baseColor, endColor, static_cast<float>(i) / static_cast<float>(radius));
			LinaVG::StyleOptions style;
			style.color = color.AsLVG4();
			lvg->DrawRect(usedRect.pos.AsLVG(), usedRect.GetEnd().AsLVG(), style, 0.0f, drawOrder);

			usedRect.size += Vector2::One * 2.0f;
			usedRect.pos -= Vector2::One * 1.0f;
		}
	}

	DirectionalLayout* WidgetUtility::BuildLayoutForPopups(Widget* source)
	{
		DirectionalLayout* popup = source->GetWidgetManager()->Allocate<DirectionalLayout>("PopupLayout");
		popup->GetFlags().Set(WF_USE_FIXED_SIZE_X | WF_SIZE_Y_TOTAL_CHILDREN);
		popup->GetChildMargins()		   = {.top = Theme::GetDef().baseIndentInner, .bottom = Theme::GetDef().baseIndentInner};
		popup->GetProps().backgroundStyle  = DirectionalLayout::BackgroundStyle::Default;
		popup->GetProps().direction		   = DirectionOrientation::Vertical;
		popup->GetProps().outlineThickness = Theme::GetDef().baseOutlineThickness;
		popup->GetProps().colorOutline	   = Theme::GetDef().silent0;
		return popup;
	}

	bool WidgetUtility::CheckIfCanShrinkWidgets(const Vector<Widget*>& widgets, float absAmount, bool isX, float minSize)
	{
		for (auto* w : widgets)
		{
			if (isX)
			{
				if (w->GetParent()->GetSizeX() * (w->GetAlignedSizeX() - absAmount) < minSize)
					return false;
			}
			else
			{
				if (w->GetParent()->GetSizeY() * (w->GetAlignedSizeY() - absAmount) < minSize)
					return false;
			}
		}
		return true;
	}

	Text* WidgetUtility::BuildEditableText(Widget* source, bool horizontal, Delegate<void()>&& onTextChanged)
	{
		Text* txt = source->GetWidgetManager()->Allocate<Text>("EditableText");

		txt->GetProps().onClicked = [onTextChanged, horizontal, source, txt]() {
			InputField* inp = source->GetWidgetManager()->Allocate<InputField>();
			inp->GetFlags().Set(WF_USE_FIXED_SIZE_Y);
			inp->GetText()->GetProps().text = txt->GetProps().text;
			inp->SetFixedSizeY(Theme::GetDef().baseItemHeight);
			inp->SetSizeX(txt->GetSizeX() * 2);
			inp->Initialize();

			if (horizontal)
			{
				inp->SetPosX(txt->GetPosX());
				inp->SetPosY(txt->GetParent()->GetPosY());
			}
			else
			{
				inp->SetPosX(txt->GetParent()->GetRect().GetCenter().x - inp->GetHalfSizeX());
				inp->SetPosY(txt->GetPosY());
			}

			inp->GetProps().onEditEnd = [onTextChanged, inp, txt, source](const String& str) {
				txt->GetProps().text = str;
				txt->CalculateTextSize();
				source->GetWidgetManager()->AddToKillList(inp);

				if (onTextChanged)
					onTextChanged();
			};

			source->GetWidgetManager()->AddToForeground(inp);
			inp->StartEditing();
			inp->SelectAll();
		};

		return txt;
	}

} // namespace Lina
