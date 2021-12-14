#include "CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace diffusion {

CameraView calculate_camera_view(entt::registry& registry, CameraComponent& camera, TransformComponent& transform)
{
    glm::mat4 global_world = calculate_global_world_matrix(registry, transform);

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(diffusion::calculate_global_world_matrix(registry, transform), scale, rotation, translation, skew, perspective);
    glm::vec3 euler = glm::eulerAngles(rotation);

    glm::mat4 rotation_matrix(1);

    glm::vec3 RotationX(1.0, 0, 0);
    rotation_matrix = glm::rotate(glm::mat4(1), euler[0], RotationX);

    glm::vec3 RotationY(0, 1.0, 0);
    rotation_matrix *= glm::rotate(glm::mat4(1), euler[1], RotationY);

    glm::vec3 RotationZ(0, 0, 1.0);
    rotation_matrix *= glm::rotate(glm::mat4(1), euler[2], RotationZ);

    glm::vec3 camera_target = rotation_matrix * glm::vec4(0, 1, 0, 1);
    glm::vec3 up_vector = rotation_matrix * glm::vec4(0, 0, -1, 1);

    return { translation, translation + camera_target, up_vector };
}

glm::vec3 get_world_point_by_screen(entt::registry & registry, float normalized_screen_x, float normalized_screen_y, double depth)
{
	auto& main_camera_entity = registry.ctx<diffusion::MainCameraTag>();
	auto& camera_component = registry.get<diffusion::CameraComponent>(main_camera_entity.m_entity);
    auto& transform = registry.get<TransformComponent>(main_camera_entity.m_entity);

    auto camera_view = calculate_camera_view(registry, camera_component, transform);

	glm::vec3 direction = camera_view.target - camera_view.position;

	glm::vec3 RotationZ{ 0, 0, 1.0 };
	glm::mat4 rotation_matrix_z = glm::rotate(glm::mat4(1.0f), camera_component.fov_x * normalized_screen_x / 2, RotationZ);
	//glm::vec3 direction_z = glm::vec3(rotation_matrix_z * glm::vec4(direction, 1.0f));

	glm::vec3 RotationX{ -1.0, 0, 0 };
    glm::mat4 rotation_matrix_xz = glm::rotate(rotation_matrix_z, camera_component.fov_y * normalized_screen_y / 2, RotationX);
	//glm::mat4 rotation_matrix_x = glm::rotate(glm::mat4(1.0f), camera_component.fov_y * normalized_screen_y / 2, RotationX);
	//glm::vec3 direction_zx = glm::vec3(rotation_matrix_x * glm::vec4(direction_z, 1.0f));
    glm::vec3 direction_zx = glm::vec3(rotation_matrix_xz * glm::vec4(direction, 1.0f));

	double min_distance_inv = 1.0 / camera_component.min_distance;
	double max_distance_inv = 1.0 / camera_component.max_distance;

	double length = 2 / (min_distance_inv + depth * (max_distance_inv - min_distance_inv));
	return camera_view.position + glm::normalize(direction_zx) * static_cast<float>(length);
}

} // namespace diffusion {