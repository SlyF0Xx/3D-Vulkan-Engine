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
        static_cast<float>(glm::radians(90.0f)),  // ������������ ���� ������ � ��������. ������ ����� 90&deg; (����� �������) � 30&deg; (�����)
        1.0f,                          // ��������� ������. ������� �� �������� ������ ����. ��������, ��� 4/3 == 800/600 == 1280/960
        0.1f,                                  // ������� ��������� ���������. ������ ���� ������ 0.
        100.0f                                 // ������� ��������� ���������.
    );
   
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PointLightComponent, m_projection_matrix)
};

} // namespace diffusion {