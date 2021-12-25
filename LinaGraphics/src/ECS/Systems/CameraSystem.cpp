/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

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

#include "ECS/Systems/CameraSystem.hpp"

#include "Core/RenderEngineBackend.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/EntityDataComponent.hpp"
#include "ECS/Registry.hpp"
#include "Log/Log.hpp"
#include "Math/Color.hpp"
#include "Math/Matrix.hpp"

namespace Lina::ECS
{

    void CameraSystem::Initialize(float aspect)
    {
        System::Initialize();
        SetAspectRatio(aspect);
        m_ecs->on_destroy<CameraComponent>().connect<&CameraSystem::OnCameraDestroyed>(this);
    }

    void CameraSystem::SetActiveCamera(Entity cameraOwner)
    {
        if (cameraOwner == entt::null)
        {
            m_activeCameraEntity = entt::null;
            return;
        }

        if (m_ecs->all_of<CameraComponent>(cameraOwner))
            m_activeCameraEntity = cameraOwner;
        else
            LINA_WARN("This entity does not have a camera component, can not set it as main camera.");
    }

    Vector3 CameraSystem::ScreenToWorldCoordinates(const Vector3& screenPos)
    {
        auto*   renderEngine = Graphics::RenderEngineBackend::Get();
        Vector2 windowSize   = renderEngine->GetScreenSize();
        Vector2 windowPos    = renderEngine->GetScreenPos();
        Vector4 viewport(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
        Vector3 win      = glm::vec3(screenPos.x, windowSize.y - screenPos.y, 1.0f);
        Matrix  pp       = renderEngine->GetCameraSystem()->GetProjectionMatrix();
        Matrix  vv       = renderEngine->GetCameraSystem()->GetViewMatrix();
        Vector3 camPos   = renderEngine->GetCameraSystem()->GetCameraLocation();
        Vector3 worldPos = glm::unProject(win, vv, pp, viewport);
        return Vector3(glm::normalize(worldPos - camPos)) * screenPos.z + camPos;
    }

    Vector3 CameraSystem::ViewportToWorldCoordinates(const Vector3& viewport)
    {
        auto*   renderEngine = Graphics::RenderEngineBackend::Get();
        Vector2 windowSize   = renderEngine->GetScreenSize();
        return ScreenToWorldCoordinates(Vector3(viewport.x * windowSize.x, viewport.y * windowSize.y, viewport.z));
    }

    Vector3 CameraSystem::WorldToScreenCoordinates(const Vector3& world)
    {
        auto*   renderEngine = Graphics::RenderEngineBackend::Get();
        Vector2 windowSize   = renderEngine->GetScreenSize();
        Vector2 windowPos    = renderEngine->GetScreenPos();
        Vector4 viewport(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
        Matrix  pp     = renderEngine->GetCameraSystem()->GetProjectionMatrix();
        Matrix  vv     = renderEngine->GetCameraSystem()->GetViewMatrix();
        Vector3 result = glm::project(world, vv, pp, viewport);
        return Vector3(result.x, windowSize.y - result.y, result.z);
    }

    Vector3 CameraSystem::WorldToViewportCoordinates(const Vector3& world)
    {
        auto*   renderEngine = Graphics::RenderEngineBackend::Get();
        Vector2 windowSize   = renderEngine->GetScreenSize();
        Vector3 screen       = WorldToScreenCoordinates(world);
        return Vector3(screen.x / windowSize.x, screen.y / windowSize.y, screen.z);
    }

    void CameraSystem::UpdateComponents(float interpolation)
    {
        if (m_activeCameraEntity != entt::null)
        {
            CameraComponent&     camera = m_ecs->get<CameraComponent>(m_activeCameraEntity);
            EntityDataComponent& data   = m_ecs->get<EntityDataComponent>(m_activeCameraEntity);

            if (!m_viewMatrixInjected)
            {
                // Actual camera view matrix.
                Vector3    location = data.GetLocation();
                Quaternion rotation = data.GetRotation();
                m_view              = Matrix::InitLookAt(location, location + rotation.GetForward(), rotation.GetUp());
            }
            else
                m_viewMatrixInjected = false;

            // Update projection matrix.
            if (!m_projMatrixInjected)
                m_projection = Matrix::Perspective(camera.m_fieldOfView / 2, m_aspectRatio, camera.m_zNear, camera.m_zFar);
            else
                m_projMatrixInjected = false;
        }
    }

    Vector3 CameraSystem::GetCameraLocation()
    {
        return m_activeCameraEntity == entt::null ? Vector3(Vector3::Zero) : m_ecs->get<EntityDataComponent>(m_activeCameraEntity).GetLocation();
    }

    Color& CameraSystem::GetCurrentClearColor()
    {
        return m_activeCameraEntity == entt::null ? Color::Gray : m_ecs->get<CameraComponent>(m_activeCameraEntity).m_clearColor;
    }

    CameraComponent* CameraSystem::GetActiveCameraComponent()
    {
        return m_activeCameraEntity == entt::null ? nullptr : m_ecs->try_get<CameraComponent>(m_activeCameraEntity);
    }

} // namespace Lina::ECS
