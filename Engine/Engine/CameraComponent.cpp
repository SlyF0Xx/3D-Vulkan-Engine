#include "CameraComponent.h"

namespace diffusion {
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