#include "CameraSystem.h"

#include "../BaseComponents/CameraComponent.h"

namespace diffusion {

CameraSystem::CameraSystem(::entt::registry& registry)
    : m_registry(registry)
{
}

void CameraSystem::move_forward(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);

        direction = glm::normalize(camera_view.target - camera_view.position) * multiplier;

        transform.m_world_matrix = glm::translate(glm::mat4(1), direction) * transform.m_world_matrix;
    });
    callback_list(direction);
}

void CameraSystem::move_backward(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);

        direction = glm::normalize(camera_view.target - camera_view.position) * multiplier;

        transform.m_world_matrix = glm::translate(glm::mat4(1), -direction) * transform.m_world_matrix;
    });
    callback_list(-direction);
}

void CameraSystem::move_left(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);

        glm::vec3 forward_vec = glm::normalize(camera_view.target - camera_view.position) * multiplier;
        direction = glm::cross(forward_vec, camera_view.up) * multiplier;

        transform.m_world_matrix = glm::translate(glm::mat4(1), -direction) * transform.m_world_matrix;
    });
    callback_list(-direction);
}

void CameraSystem::move_right(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);

        glm::vec3 forward_vec = glm::normalize(camera_view.target - camera_view.position) * multiplier;
        direction = glm::cross(forward_vec, camera_view.up) * multiplier;

        transform.m_world_matrix = glm::translate(glm::mat4(1), direction) * transform.m_world_matrix;
    });
    callback_list(direction);
}

void CameraSystem::move_up(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);
        direction = glm::vec3(camera_view.up * multiplier);

        transform.m_world_matrix = glm::translate(glm::mat4(1), direction) * transform.m_world_matrix;
    });
    callback_list(direction);
}

void CameraSystem::move_down(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<TransformComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](TransformComponent& transform) {
        auto& camera = m_registry.get<CameraComponent>(entt::to_entity(m_registry, transform));
        auto camera_view = calculate_camera_view(m_registry, camera, transform);
        direction = glm::vec3(camera_view.up * multiplier);

        transform.m_world_matrix = glm::translate(glm::mat4(1), -direction) * transform.m_world_matrix;
    });
    callback_list(-direction);
}

} // namespace diffusion {