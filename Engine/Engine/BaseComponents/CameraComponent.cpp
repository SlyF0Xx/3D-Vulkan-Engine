#include "CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

CameraComponent create_camera_component()
{
    CameraComponent camera;
    camera.m_projection_matrix = glm::perspective(
        static_cast<float>(glm::radians(60.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
        16.0f / 9.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
        0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
        100.0f                                 // Дальняя плоскость отсечения.
    );

    //recalculate_state();
    camera.m_camera_matrix = glm::lookAt(
        camera.m_camera_position, // Позиция камеры в мировом пространстве
        camera.m_camera_target,   // Указывает куда вы смотрите в мировом пространстве
        camera.m_up_vector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );

    return camera;
}

glm::vec3 get_world_point_by_screen(entt::registry& registry, float normalized_screen_x, float normalized_screen_y, double depth) {
    auto& main_camera_entity = registry.ctx<diffusion::MainCameraTag>();
    auto& camera_component = registry.get<diffusion::CameraComponent>(main_camera_entity.m_entity);

    glm::vec3 direction = camera_component.m_camera_target - camera_component.m_camera_position;

    glm::vec3 RotationZ {0, 0, 1.0};
    glm::mat4 rotation_matrix_z = glm::rotate(glm::mat4(1.0f), camera_component.fov_x * normalized_screen_x / 2, RotationZ);
    glm::vec3 direction_z = glm::vec3(rotation_matrix_z * glm::vec4(direction, 1.0f));

    glm::vec3 RotationX {1.0, 0, 0};
    glm::mat4 rotation_matrix_x = glm::rotate(glm::mat4(1.0f), camera_component.fov_y * normalized_screen_y / 2, RotationX);
    glm::vec3 direction_zx = glm::vec3(rotation_matrix_x * glm::vec4(direction_z, 1.0f));

    double min_distance_inv = 1.0 / camera_component.min_distance;
    double max_distance_inv = 1.0 / camera_component.max_distance;

    double length = 2 / (min_distance_inv + depth * (max_distance_inv - min_distance_inv));
    return camera_component.m_camera_position + glm::normalize(direction_zx) * static_cast<float>(length);
}

} // namespace diffusion {