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
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        direction = glm::normalize(camera.m_camera_target - camera.m_camera_position) * multiplier;
        camera.m_camera_position += direction;
        camera.m_camera_target += direction;
    });
    callback_list(direction);
}

void CameraSystem::move_backward(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        direction = glm::normalize(camera.m_camera_target - camera.m_camera_position) * multiplier;
        camera.m_camera_position -= direction;
        camera.m_camera_target -= direction;
    });
    callback_list(-direction);
}

void CameraSystem::move_left(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        glm::vec3 forward_vec = glm::normalize(camera.m_camera_target - camera.m_camera_position);
        direction = glm::cross(forward_vec, camera.m_up_vector) * multiplier;
        camera.m_camera_position -= direction;
        camera.m_camera_target -= direction;
    });
    callback_list(-direction);
}

void CameraSystem::move_right(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        glm::vec3 forward_vec = glm::normalize(camera.m_camera_target - camera.m_camera_position);
        direction = glm::cross(forward_vec, camera.m_up_vector) * multiplier;
        camera.m_camera_position += direction;
        camera.m_camera_target += direction;
    });
    callback_list(direction);
}

void CameraSystem::move_up(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        direction = glm::vec3(camera.m_up_vector * multiplier);
        camera.m_camera_position += direction;
        camera.m_camera_target += direction;
    });
    callback_list(direction);
}

void CameraSystem::move_down(float multiplier)
{
    glm::vec3 direction;
    m_registry.patch<CameraComponent>(m_registry.ctx<MainCameraTag>().m_entity, [this, multiplier, &direction](CameraComponent& camera) {
        direction = glm::vec3(camera.m_up_vector * multiplier);
        camera.m_camera_position -= direction;
        camera.m_camera_target -= direction;
    });
    callback_list(-direction);
}

} // namespace diffusion {