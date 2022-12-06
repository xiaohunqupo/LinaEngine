#include "Core/ShortcutManager.hpp"
#include "Core/InputMappings.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/InputEvents.hpp"
#include "Core/EditorCommon.hpp"

namespace Lina::Editor
{
    void ShortcutManager::Initialize()
    {
        Event::EventSystem::Get()->Connect<Event::EKeyCallback, &ShortcutManager::OnKeyCallback>(this);
        Event::EventSystem::Get()->Connect<Event::EMouseButtonCallback, &ShortcutManager::OnMouseCallback>(this);

        // RegisterShortcut(EShortcut{"ReloadGUI", LINA_KEY_LCTRL, LINA_KEY_R});
    }

    void ShortcutManager::RegisterShortcut(EShortcut shortcut)
    {
        m_shortcuts.push_back(std::make_pair(shortcut, false));
    }

    void ShortcutManager::OnKeyCallback(const Event::EKeyCallback& ev)
    {
        for (auto& shortcut : m_shortcuts)
        {
            if (ev.action == Input::InputAction::Pressed)
            {
                if (shortcut.first.heldKey == ev.key)
                    shortcut.second = true;
                else if (shortcut.first.pressedKey == ev.key && shortcut.second)
                {
                    Event::EventSystem::Get()->Trigger<EShortcut>(shortcut.first);
                }
            }
            else if (ev.action == Input::InputAction::Released)
            {
                if (shortcut.first.heldKey == ev.key && shortcut.second)
                    shortcut.second = false;
            }
        }
    }

    void ShortcutManager::OnMouseCallback(const Event::EMouseButtonCallback& ev)
    {
    }

} // namespace Lina::Editor