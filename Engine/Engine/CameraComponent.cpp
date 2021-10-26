#include "CameraComponent.h"

namespace diffusion {

CameraComponent::CameraComponent(Game& game, const std::vector<Tag>& tags, Entity* parent)
    : Component(concat_vectors({ s_camera_component_tag }, tags), parent), m_game(game)
{
    m_projection_matrix = glm::perspective(
        static_cast<float>(glm::radians(60.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
        16.0f / 9.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
        0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
        100.0f                                 // Дальняя плоскость отсечения.
    );

    //recalculate_state();
    m_camera_matrix = glm::lookAt(
        m_camera_position, // Позиция камеры в мировом пространстве
        m_camera_target,   // Указывает куда вы смотрите в мировом пространстве
        m_up_vector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );
}

void CameraComponent::move_forward(float multiplier)
{
    glm::vec3 direction = glm::normalize(m_camera_target - m_camera_position) * multiplier;
    m_camera_position += direction;
    m_camera_target += direction;
    recalculate_state();

    callback_list(direction);
}

void CameraComponent::move_backward(float multiplier)
{
    glm::vec3 direction = glm::normalize(m_camera_target - m_camera_position) * multiplier;
    m_camera_position -= direction;
    m_camera_target -= direction;
    recalculate_state();

    callback_list(-direction);
}

void CameraComponent::move_left(float multiplier)
{
    glm::vec3 forward_vec = glm::normalize(m_camera_target - m_camera_position);
    glm::vec3 direction = glm::cross(forward_vec, m_up_vector) * multiplier;
    m_camera_position -= direction;
    m_camera_target -= direction;
    recalculate_state();

    callback_list(-direction);
}

void CameraComponent::move_right(float multiplier)
{
    glm::vec3 forward_vec = glm::normalize(m_camera_target - m_camera_position);
    glm::vec3 direction = glm::cross(forward_vec, m_up_vector) * multiplier;
    m_camera_position += direction;
    m_camera_target += direction;
    recalculate_state();

    callback_list(direction);
}

void CameraComponent::move_up(float multiplier)
{
    glm::vec3 direction = glm::vec3(m_up_vector * multiplier);
    m_camera_position += direction;
    m_camera_target += direction;
    recalculate_state();

    callback_list(direction);
}

void CameraComponent::move_down(float multiplier)
{
    glm::vec3 direction = glm::vec3(m_up_vector * multiplier);
    m_camera_position -= direction;
    m_camera_target -= direction;
    recalculate_state();

    callback_list(-direction);
}

void CameraComponent::recalculate_state()
{
    m_camera_matrix = glm::lookAt(
        m_camera_position, // Позиция камеры в мировом пространстве
        m_camera_target,   // Указывает куда вы смотрите в мировом пространстве
        m_up_vector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );

    // m_game.update_camera_projection_matrixes(m_camera_matrix, m_projection_matrix);
}

namespace entt {

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

}

} // namespace diffusion {