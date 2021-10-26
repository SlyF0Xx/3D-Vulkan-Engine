#pragma once

#include "Engine.h"
#include "glm_printer.h"

#include "eventpp/callbacklist.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

struct MainCameraTag
{
	::entt::entity m_entity;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MainCameraTag, m_entity)
};

struct CameraComponent
{
	glm::vec3 m_camera_position{ 0.0f, -10.0f, 0.0f };
	glm::vec3 m_camera_target{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_up_vector{ 0.0f, 0.0f, -1.0f };

	glm::mat4 m_camera_matrix = glm::lookAt(
		m_camera_position, // Позиция камеры в мировом пространстве
		m_camera_target,   // Указывает куда вы смотрите в мировом пространстве
		m_up_vector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
	);
	glm::mat4 m_projection_matrix = glm::perspective(
		static_cast<float>(glm::radians(60.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
		16.0f / 9.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
		0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
		100.0f                                 // Дальняя плоскость отсечения.
	);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CameraComponent, m_camera_position, m_camera_target, m_up_vector, m_camera_matrix, m_projection_matrix)
};

}

} // namespace diffusion {