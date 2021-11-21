#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

struct PointLightComponent
{
    // TODO: light color, etc

    glm::mat4 m_projection_matrix =
        glm::perspective(
        static_cast<float>(glm::radians(90.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
        1.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
        0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
        100.0f                                 // Дальняя плоскость отсечения.
    );
   
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PointLightComponent, m_projection_matrix)
};

} // namespace diffusion {