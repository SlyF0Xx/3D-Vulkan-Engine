#include "CameraComponent.h"

namespace diffusion {
namespace entt {

CameraComponent create_camera_component()
{
    CameraComponent camera;
    camera.m_projection_matrix = glm::perspective(
        static_cast<float>(glm::radians(60.0f)),  // ������������ ���� ������ � ��������. ������ ����� 90&deg; (����� �������) � 30&deg; (�����)
        16.0f / 9.0f,                          // ��������� ������. ������� �� �������� ������ ����. ��������, ��� 4/3 == 800/600 == 1280/960
        0.1f,                                  // ������� ��������� ���������. ������ ���� ������ 0.
        100.0f                                 // ������� ��������� ���������.
    );

    //recalculate_state();
    camera.m_camera_matrix = glm::lookAt(
        camera.m_camera_position, // ������� ������ � ������� ������������
        camera.m_camera_target,   // ��������� ���� �� �������� � ������� ������������
        camera.m_up_vector        // ������, ����������� ����������� �����. ������ (0, 1, 0)
    );

    return camera;
}

}

} // namespace diffusion {