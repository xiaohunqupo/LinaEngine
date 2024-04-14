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

#include "Core/GUI/Widgets/WidgetManager.hpp"
#include "Core/GUI/Widgets/Widget.hpp"
#include "Core/GUI/Widgets/Compound/Popup.hpp"
#include "Core/GUI/Widgets/Layout/ScrollArea.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/Graphics/Resource/Font.hpp"
#include "Core/Graphics/GfxManager.hpp"
#include "Core/CommonCore.hpp"
#include "Common/Data/CommonData.hpp"
#include "Common/System/System.hpp"
#include "Common/Platform/LinaGXIncl.hpp"
#include "Common/Platform/LinaVGIncl.hpp"
#include "Common/Math/Math.hpp"
#include "LinaGX/Core/InputMappings.hpp"

namespace Lina
{

	void WidgetManager::Initialize(System* system, LinaGX::Window* window)
	{
		m_window	 = window;
		m_system	 = system;
		m_gfxManager = m_system->CastSubsystem<GfxManager>(SubsystemType::GfxManager);
		m_window->AddListener(this);
		m_rootWidget = Allocate<Widget>();
		m_rootWidget->SetDebugName("Root");

		m_foregroundRoot = Allocate<Widget>();
		m_foregroundRoot->SetDebugName("ForegroundRoot");

		m_resourceManager = m_system->CastSubsystem<ResourceManager>(SubsystemType::ResourceManager);
		m_defaultFont	  = m_resourceManager->GetResource<Font>(Theme::GetDef().defaultFont);
	}

	void WidgetManager::PreTick()
	{
		for (auto* w : m_killList)
		{
			w->GetParent()->RemoveChild(w);
			Deallocate(w);
		}

		m_killList.clear();

		if (m_foregroundRoot->GetChildren().empty())
			m_window->SetCursorType(FindCursorType(m_rootWidget));
		else
			m_window->SetCursorType(FindCursorType(m_foregroundRoot));

		m_foregroundRoot->SetDrawOrder(FOREGROUND_DRAW_ORDER);
		PassPreTick(m_foregroundRoot);
		PassPreTick(m_rootWidget);
	}

	void WidgetManager::Tick(float delta, const Vector2ui& size)
	{
		// actual tick.
		m_debugDrawYOffset = 0.0f;
		m_foregroundRoot->SetPos(Vector2::Zero);
		m_foregroundRoot->SetSize(Vector2(static_cast<float>(size.x), static_cast<float>(size.y)));
		m_rootWidget->SetPos(Vector2::Zero);
		m_rootWidget->SetSize(Vector2(static_cast<float>(size.x), static_cast<float>(size.y)));

		PassCalculateSize(m_foregroundRoot, delta);
		PassTick(m_foregroundRoot, delta);

		PassCalculateSize(m_rootWidget, delta);
		PassTick(m_rootWidget, delta);
	}

	void WidgetManager::AddToForeground(Widget* w)
	{
		m_foregroundRoot->AddChild(w);
	}

	void WidgetManager::RemoveFromForeground(Widget* w)
	{
		m_foregroundRoot->RemoveChild(w);
	}

	void WidgetManager::Draw(int32 threadIndex)
	{
		m_rootWidget->Draw(threadIndex);

		if (!m_foregroundRoot->GetChildren().empty())
		{
			LinaVG::StyleOptions opts;
			opts.color = LinaVG::Vec4(0.0f, 0.0f, 0.0f, m_foregroundDim);
			LinaVG::DrawRect(threadIndex, Vector2::Zero.AsLVG(), Vector2(static_cast<float>(m_window->GetSize().x), static_cast<float>(m_window->GetSize().y)).AsLVG(), opts, 0.0f, FOREGROUND_DRAW_ORDER);
			m_foregroundRoot->Draw(threadIndex);
		}

		if (!m_foregroundRoot->GetChildren().empty())
			DebugDraw(threadIndex, m_foregroundRoot);
		else
			DebugDraw(threadIndex, m_rootWidget);
	}

	void WidgetManager::Deallocate(Widget* widget)
	{
		// If the widget is the active controls owner of a manager
		if (widget->GetLocalControlsManager() && widget->GetLocalControlsManager()->GetLocalControlsOwner() == widget)
			widget->GetLocalControlsManager()->SetLocalControlsOwner(nullptr);

		// If the widget is the manager of someone
		if (widget->GetLocalControlsOwner())
			widget->GetLocalControlsOwner()->SetLocalControlsManager(nullptr);

		if (GetControlsOwner() == widget)
			ReleaseControls(widget);

		if (m_lastControlsManager == widget)
			m_lastControlsManager = nullptr;

		widget->m_customTooltip = nullptr;

		for (auto* c : widget->m_children)
			Deallocate(c);

		const TypeID tid = widget->m_tid;
		widget->Destruct();

		GetGUIAllocator(tid, 0)->Free(widget);
	}

	PoolAllocator* WidgetManager::GetGUIAllocator(TypeID tid, size_t typeSize)
	{
		return m_gfxManager->GetGUIAllocator(tid, typeSize);
	}

	void WidgetManager::Shutdown()
	{
		Deallocate(m_rootWidget);
		Deallocate(m_foregroundRoot);
		m_rootWidget	 = nullptr;
		m_foregroundRoot = nullptr;

		m_window->RemoveListener(this);
	}

	void WidgetManager::OnWindowKey(uint32 keycode, int32 scancode, LinaGX::InputAction inputAction)
	{
		if (keycode == LINAGX_KEY_TAB && inputAction != LinaGX::InputAction::Released && m_controlsOwner != nullptr)
		{
			if (m_window->GetInput()->GetKey(LINAGX_KEY_LSHIFT))
				MoveControlsToPrev();
			else
				MoveControlsToNext();
			return;
		}

		if (PassKey(m_foregroundRoot, keycode, scancode, inputAction))
			return;

		PassKey(m_rootWidget, keycode, scancode, inputAction);
	}

	void WidgetManager::OnWindowMouse(uint32 button, LinaGX::InputAction inputAction)
	{
		// If we have some items in the foreground
		// check if any was clicked, if not, then remove the non-blocker ones
		// this is used for removing popups mostly.
		if (button == LINAGX_MOUSE_0 && inputAction == LinaGX::InputAction::Pressed && !m_foregroundRoot->GetChildren().empty())
		{
			bool anyForegroundHovered = false;
			for (auto* c : m_foregroundRoot->GetChildren())
			{
				if (c->GetIsHovered())
				{
					anyForegroundHovered = true;
					break;
				}
			}

			if (!anyForegroundHovered)
			{
				Vector<Widget*> removeList;
				for (auto* c : m_foregroundRoot->GetChildren())
				{
					if (!c->GetFlags().IsSet(WF_FOREGROUND_BLOCKER))
						removeList.push_back(c);
				}

				for (auto* w : removeList)
				{
					RemoveFromForeground(w);
					Deallocate(w);
				}

				return;
			}
		}

		// Left click presses to anywhere outside the control owner
		// releases controls from that owner.
		if (button == LINAGX_MOUSE_0 && inputAction == LinaGX::InputAction::Pressed && m_controlsOwner != nullptr && !m_controlsOwner->GetIsHovered())
		{
			if (m_controlsOwner->GetLocalControlsManager())
			{
				if (m_controlsOwner->GetLocalControlsManager()->GetIsHovered())
					m_controlsOwner->GetLocalControlsManager()->SetLocalControlsOwner(nullptr);
			}

			ReleaseControls(m_controlsOwner);
		}

		if (PassMouse(m_foregroundRoot, button, inputAction))
			return;

		PassMouse(m_rootWidget, button, inputAction);
	}

	void WidgetManager::OnWindowMouseWheel(float amt)
	{
		if (PassMouseWheel(m_foregroundRoot, amt))
			return true;
		PassMouseWheel(m_rootWidget, amt);
	}

	void WidgetManager::OnWindowMouseMove(const LinaGX::LGXVector2& pos)
	{
	}

	void WidgetManager::OnWindowFocus(bool gainedFocus)
	{
	}

	void WidgetManager::OnWindowHoverBegin()
	{
	}

	void WidgetManager::OnWindowHoverEnd()
	{
	}

	void WidgetManager::DebugDraw(int32 threadIndex, Widget* w)
	{
		const bool drawRects = (m_window->GetInput()->GetMouseButton(LINAGX_MOUSE_1));

		if (!drawRects)
			return;

		for (auto* c : w->m_children)
			DebugDraw(threadIndex, c);

		LinaVG::StyleOptions opts;
		opts.isFilled  = false;
		opts.thickness = 2.0f;

		const Rect			rect	= w->GetRect();
		const Vector2&		mp		= m_window->GetMousePosition();
		auto*				lvgFont = m_resourceManager->GetResource<Font>(DEFAULT_FONT_SID)->GetLinaVGFont(m_window->GetDPIScale());
		LinaVG::TextOptions textOpts;
		textOpts.font = lvgFont;

		if (drawRects)
			LinaVG::DrawRect(threadIndex, rect.pos.AsLVG(), (rect.pos + rect.size).AsLVG(), opts, 0.0f, 1000.0f);

		if (w->m_isHovered)
		{
			const Vector2 sz = LinaVG::CalculateTextSize(w->GetDebugName().c_str(), textOpts);
			LinaVG::DrawTextNormal(threadIndex, w->GetDebugName().c_str(), (mp + Vector2(15, 15 + m_debugDrawYOffset)).AsLVG(), textOpts, 0.0f, DEBUG_DRAW_ORDER);

			const String  rectStr = "Pos: (" + UtilStr::FloatToString(w->GetPos().x, 1) + ", " + UtilStr::FloatToString(w->GetPos().y, 1) + ") Size: (" + UtilStr::FloatToString(w->GetSize().x, 1) + ", " + UtilStr::FloatToString(w->GetSize().y, 1) + ")";
			const Vector2 sz2	  = LinaVG::CalculateTextSize(rectStr.c_str(), textOpts);
			LinaVG::DrawTextNormal(threadIndex, rectStr.c_str(), (mp + Vector2(15 + sz.x + 15, 15 + m_debugDrawYOffset)).AsLVG(), textOpts, 0.0f, DEBUG_DRAW_ORDER);

			const String drawOrder = "DO: " + TO_STRING(w->GetDrawOrder());
			LinaVG::DrawTextNormal(threadIndex, drawOrder.c_str(), (mp + Vector2(15 + sz.x + 15 + sz2.x + 15, 15 + m_debugDrawYOffset)).AsLVG(), textOpts, 0.0f, DEBUG_DRAW_ORDER);
			m_debugDrawYOffset += lvgFont->m_size * 1.5f;
		}

		if (drawRects)
			w->DebugDraw(threadIndex, DEBUG_DRAW_ORDER);
	}

	void WidgetManager::SetClip(int32 threadIndex, const Rect& r, const TBLR& margins)
	{
		const ClipData cd = {
			.rect	 = r,
			.margins = margins,
		};

		m_clipStack.push_back(cd);

		LinaVG::SetClipPosX(static_cast<uint32>(r.pos.x + margins.left), threadIndex);
		LinaVG::SetClipPosY(static_cast<uint32>(r.pos.y + margins.top), threadIndex);
		LinaVG::SetClipSizeX(static_cast<uint32>(r.size.x - (margins.left + margins.right)), threadIndex);
		LinaVG::SetClipSizeY(static_cast<uint32>(r.size.y - (margins.top + margins.bottom)), threadIndex);
	}

	void WidgetManager::UnsetClip(int32 threadIndex)
	{
		m_clipStack.pop_back();

		if (m_clipStack.empty())
		{
			LinaVG::SetClipPosX(0, threadIndex);
			LinaVG::SetClipPosY(0, threadIndex);
			LinaVG::SetClipSizeX(0, threadIndex);
			LinaVG::SetClipSizeY(0, threadIndex);
		}
		else
		{
			const ClipData& cd = m_clipStack[m_clipStack.size() - 1];
			LinaVG::SetClipPosX(static_cast<uint32>(cd.rect.pos.x + cd.margins.left), threadIndex);
			LinaVG::SetClipPosY(static_cast<uint32>(cd.rect.pos.y + cd.margins.top), threadIndex);
			LinaVG::SetClipSizeX(static_cast<uint32>(cd.rect.size.x - (cd.margins.left + cd.margins.right)), threadIndex);
			LinaVG::SetClipSizeY(static_cast<uint32>(cd.rect.size.y - (cd.margins.top + cd.margins.bottom)), threadIndex);
		}
	}

	void WidgetManager::AddToKillList(Widget* w)
	{
		m_killList.push_back(w);
	}

	LinaGX::CursorType WidgetManager::FindCursorType(Widget* w)
	{
		const LinaGX::CursorType cursorType = w->GetCursorOverride();
		if (cursorType != LinaGX::CursorType::Default)
		{
			return cursorType;
		}

		const Vector<Widget*> children = w->GetChildren();
		for (Widget* c : children)
		{
			LinaGX::CursorType childCursorType = FindCursorType(c);
			if (childCursorType != LinaGX::CursorType::Default)
			{
				return childCursorType;
			}
		}
		return LinaGX::CursorType::Default;
	}

	namespace
	{
		float CalculateAlignedPosX(Widget* w)
		{
			const Vector2			 alignedPos = w->GetAlignedPos();
			const PosAlignmentSource src		= w->GetPosAlignmentSourceX();
			const Vector2			 start		= w->GetParent()->GetStartFromMargins();
			const Vector2			 end		= w->GetParent()->GetEndFromMargins();
			const Vector2			 sz			= end - start;
			if (src == PosAlignmentSource::Center)
				return start.x + sz.x * alignedPos.x - w->GetHalfSizeX();
			else if (src == PosAlignmentSource::End)
				return start.x + sz.x * alignedPos.x - w->GetSizeX();

			return start.x + sz.x * alignedPos.x;
		}

		float CalculateAlignedPosY(Widget* w)
		{
			const Vector2			 alignedPos = w->GetAlignedPos();
			const PosAlignmentSource src		= w->GetPosAlignmentSourceY();
			const Vector2			 start		= w->GetParent()->GetStartFromMargins();
			const Vector2			 end		= w->GetParent()->GetEndFromMargins();
			const Vector2			 sz			= end - start;
			if (src == PosAlignmentSource::Center)
				return start.y + sz.y * alignedPos.y - w->GetHalfSizeY();
			else if (src == PosAlignmentSource::End)
				return start.y + sz.y * alignedPos.y - w->GetSizeY();

			return start.y + sz.y * alignedPos.y;
		}
	} // namespace

	bool WidgetManager::PassKey(Widget* widget, uint32 keycode, int32 scancode, LinaGX::InputAction inputAction)
	{
		if (!widget->GetIsDisabled() && widget->GetIsVisible() && widget->OnKey(keycode, scancode, inputAction) && !widget->GetFlags().IsSet(WF_INPUT_PASSTHRU))
			return true;

		for (auto* c : widget->GetChildren())
		{
			if (PassKey(c, keycode, scancode, inputAction))
				return true;
		}

		return false;
	}

	bool WidgetManager::PassMouse(Widget* widget, uint32 button, LinaGX::InputAction inputAction)
	{
		if (!widget->GetIsDisabled() && widget->GetIsVisible() && widget->OnMouse(button, inputAction) && !widget->GetFlags().IsSet(WF_INPUT_PASSTHRU))
			return true;

		for (auto* c : widget->GetChildren())
		{
			if (PassMouse(c, button, inputAction))
				return true;
		}

		return false;
	}

	bool WidgetManager::PassMouseWheel(Widget* widget, float amt)
	{
		if (!widget->GetIsDisabled() && widget->GetIsVisible() && widget->OnMouseWheel(amt) && !widget->GetFlags().IsSet(WF_INPUT_PASSTHRU))
			return true;

		for (auto* c : widget->GetChildren())
		{
			if (PassMouseWheel(c, amt))
				return true;
		}

		return false;
	}

	void WidgetManager::PassPreTick(Widget* w)
	{
		for (auto cb : w->m_executeNextFrame)
			cb();

		w->m_executeNextFrame.clear();

		if (!w->GetFlags().IsSet(WF_CONTROLS_DRAW_ORDER) && w->GetParent())
			w->SetDrawOrder(w->GetParent()->GetDrawOrder());

		w->SetIsHovered();
		w->PreTick();
		for (auto* c : w->GetChildren())
			PassPreTick(c);
	}

	void WidgetManager::PassCalculateSize(Widget* w, float delta)
	{
		if (!w->GetFlags().IsSet(WF_SIZE_AFTER_CHILDREN))
			w->CalculateSize(delta);

		Vector<Widget*> expandingChildren;
		Vector2			totalNonExpandingSize = Vector2::Zero;

		for (auto* c : w->GetChildren())
		{

			if (c->GetFlags().IsSet(WF_USE_FIXED_SIZE_X))
				c->SetSizeX(c->GetFixedSizeX() * m_window->GetDPIScale());

			if (c->GetFlags().IsSet(WF_USE_FIXED_SIZE_Y))
				c->SetSizeY(c->GetFixedSizeY() * m_window->GetDPIScale());

			c->CalculateSize(delta);

			const Vector2 alignedSize = c->GetAlignedSize();

			bool isExpandingX = false;
			bool isExpandingY = false;

			if (c->GetFlags().IsSet(WF_SIZE_ALIGN_X))
			{
				if (!Math::Equals(alignedSize.x, 0.0f, 0.0001f))
					c->SetSizeX((w->GetSizeX() - (w->GetChildMargins().left + w->GetChildMargins().right)) * alignedSize.x);
				else
					isExpandingX = true;
			}

			if (c->GetFlags().IsSet(WF_SIZE_ALIGN_Y))
			{
				if (!Math::Equals(alignedSize.y, 0.0f, 0.0001f))
					c->SetSizeY((w->GetSizeY() - (w->GetChildMargins().top + w->GetChildMargins().bottom)) * alignedSize.y);
				else
					isExpandingY = true;
			}

			if (c->GetFlags().IsSet(WF_SIZE_X_COPY_Y))
				c->SetSizeX(c->GetSizeY());

			if (c->GetFlags().IsSet(WF_SIZE_Y_COPY_X))
				c->SetSizeY(c->GetSizeX());

			if (!c->GetFlags().IsSet(WF_SKIP_FLOORING))
			{
				c->SetSizeX(Math::FloorToFloatEven(c->GetSizeX()));
				c->SetSizeY(Math::FloorToFloatEven(c->GetSizeY()));
			}

			if (!isExpandingX)
				totalNonExpandingSize.x += c->GetSizeX();

			if (!isExpandingY)
				totalNonExpandingSize.y += c->GetSizeY();

			if (isExpandingX || isExpandingY)
				expandingChildren.push_back(c);
		}

		if (w->GetFlags().IsSet(WF_SIZE_AFTER_CHILDREN))
			w->CalculateSize(delta);

		if (w->GetFlags().IsSet(WF_SIZE_X_TOTAL_CHILDREN))
		{
			float total = 0.0f;
			int32 idx	= 0;
			for (auto* c : w->GetChildren())
			{
				if (idx != 0)
					total += w->GetChildPadding();

				total += c->GetSizeX();
				idx++;
			}

			w->SetSizeX(total + w->GetChildMargins().left + w->GetChildMargins().right);
		}

		if (w->GetFlags().IsSet(WF_SIZE_Y_TOTAL_CHILDREN))
		{
			float total = 0.0f;
			int32 idx	= 0;
			for (auto* c : w->GetChildren())
			{
				if (idx != 0)
					total += w->GetChildPadding();

				total += c->GetSizeY();
				idx++;
			}

			w->SetSizeY(total + w->GetChildMargins().top + w->GetChildMargins().bottom);
		}

		if (w->GetFlags().IsSet(WF_SIZE_X_MAX_CHILDREN))
		{
			float max = 0.0f;
			for (auto* c : w->GetChildren())
				max = Math::Max(max, c->GetSizeX());
			w->SetSizeX(max + w->GetChildMargins().left + w->GetChildMargins().right);
		}

		if (w->GetFlags().IsSet(WF_SIZE_Y_MAX_CHILDREN))
		{
			float max = 0.0f;
			for (auto* c : w->GetChildren())
				max = Math::Max(max, c->GetSizeY());
			w->SetSizeY(max + w->GetChildMargins().top + w->GetChildMargins().bottom);
		}

		if (!expandingChildren.empty())
		{
			const Vector2 start						 = w->GetStartFromMargins();
			const Vector2 end						 = w->GetEndFromMargins();
			const Vector2 size						 = end - start;
			const Vector2 totalAvailableSpace		 = size - totalNonExpandingSize;
			const float	  totalPad					 = w->GetChildPadding() * static_cast<float>(w->GetChildren().size() - 1);
			const Vector2 totalAvailableAfterPadding = totalAvailableSpace - Vector2(totalPad, totalPad);
			const Vector2 targetSize				 = totalAvailableAfterPadding / (static_cast<float>(expandingChildren.size()));

			for (auto* c : expandingChildren)
			{
				const bool skipFlooring = c->GetFlags().IsSet(WF_SKIP_FLOORING);

				if (c->GetFlags().IsSet(WF_SIZE_ALIGN_X) && Math::Equals(c->GetAlignedSizeX(), 0.0f, 0.0001f))
				{
					if (skipFlooring)
						c->SetSizeX(targetSize.x);
					else
						c->SetSizeX(Math::FloorToFloatEven(targetSize.x));
				}

				if (c->GetFlags().IsSet(WF_SIZE_ALIGN_Y) && Math::Equals(c->GetAlignedSizeY(), 0.0f, 0.0001f))
				{
					if (skipFlooring)
						c->SetSizeY(targetSize.y);
					else
						c->SetSizeY(Math::FloorToFloatEven(targetSize.y));
				}
			}
		}

		if (!w->GetFlags().IsSet(WF_SKIP_FLOORING))
		{
			w->SetSizeX(Math::FloorToFloatEven(w->GetSizeX()));
			w->SetSizeY(Math::FloorToFloatEven(w->GetSizeY()));
		}

		for (auto* c : w->GetChildren())
			PassCalculateSize(c, delta);
	}

	void WidgetManager::PassTick(Widget* w, float delta)
	{
		if (w->GetFlags().IsSet(WF_POS_ALIGN_X) && w->GetParent())
			w->SetPosX(CalculateAlignedPosX(w));

		if (w->GetFlags().IsSet(WF_POS_ALIGN_Y) && w->GetParent())
			w->SetPosY(CalculateAlignedPosY(w));

		if (!w->GetFlags().IsSet(WF_TICK_AFTER_CHILDREN))
			w->Tick(delta);

		if (!w->GetFlags().IsSet(WF_SKIP_FLOORING))
		{
			w->SetPosX(Math::FloorToFloatEven(w->GetPosX()));
			w->SetPosY(Math::FloorToFloatEven(w->GetPosY()));
		}

		for (auto* c : w->GetChildren())
			PassTick(c, delta);

		if (w->GetFlags().IsSet(WF_TICK_AFTER_CHILDREN))
			w->Tick(delta);
	}

	ScrollArea* WidgetManager::FindScrollAreaAbove(Widget* w)
	{
		auto* parent = w->GetParent();
		if (parent)
		{

			if (parent->GetTID() == GetTypeID<ScrollArea>())
				return static_cast<ScrollArea*>(parent);

			return FindScrollAreaAbove(parent);
		}
		return nullptr;
	}

	void WidgetManager::GrabControls(Widget* widget)
	{
		m_controlsOwner = widget;

		if (widget->m_onGrabbedControls)
			widget->m_onGrabbedControls();

		auto* owningScroll = FindScrollAreaAbove(widget);

		if (owningScroll)
			owningScroll->ScrollToChild(widget);

		if (widget->GetLocalControlsManager())
			widget->GetLocalControlsManager()->SetLocalControlsOwner(widget);
	}

	void WidgetManager::ReleaseControls(Widget* widget)
	{
		if (m_controlsOwner == widget)
			m_controlsOwner = nullptr;
	}

	Widget* WidgetManager::GetControlsOwner()
	{
		return m_controlsOwner;
	}

	Widget* WidgetManager::FindNextSelectable(Widget* start)
	{
		if (!start)
			return nullptr;

		Widget* current = start;
		do
		{
			// Depth-first search for the next selectable widget
			if (!current->m_children.empty())
				current = current->m_children[0];
			else
			{
				while (current != nullptr && current->m_next == nullptr)
					current = current->m_parent;

				if (current != nullptr)
					current = current->m_next;
			}

			if (current && current->GetFlags().IsSet(WF_CONTROLLABLE) && !current->GetIsDisabled() && current->GetIsVisible())
				return current;
		} while (current != nullptr && current != start);

		return nullptr;
	}

	Widget* WidgetManager::FindPreviousSelectable(Widget* start)
	{
		if (!start)
			return nullptr;

		Widget* current = start;
		do
		{
			// Reverse depth-first search for the previous selectable widget
			if (current->m_prev)
			{
				current = current->m_prev;
				while (!current->m_children.empty())
					current = current->m_children.back();
			}
			else
				current = current->m_parent;

			if (current && current->GetFlags().IsSet(WF_CONTROLLABLE) && !current->GetIsDisabled() && current->GetIsVisible())
				return current;

		} while (current != nullptr && current != start);

		return nullptr;
	}

	void WidgetManager::MoveControlsToPrev()
	{
		if (!m_controlsOwner)
			return;

		Widget* previous = FindPreviousSelectable(m_controlsOwner);
		if (previous)
		{
			GrabControls(previous);
		}
	}

	void WidgetManager::MoveControlsToNext()
	{
		if (!m_controlsOwner)
			return;

		Widget* next = FindNextSelectable(m_controlsOwner);
		if (next)
		{
			GrabControls(next);
		}
	}

} // namespace Lina
