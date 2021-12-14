#pragma once

#include "../Engine.h"
#include "../glm_printer.h"

#include "eventpp/callbacklist.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

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

	float aspect = 16.0f / 9.0f;
	float fov_y = glm::radians(60.0f);
	float fov_x = atan(tan(fov_y / 2) * aspect) * 2;

	float min_distance = 0.1f;
	float max_distance = 100.0f;

	glm::mat4 m_projection_matrix = //glm::orthoRH_ZO(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
		glm::perspective(
		fov_y,				// ������������ ���� ������ � ��������. ������ ����� 90&deg; (����� �������) � 30&deg; (�����)
		aspect,             // ��������� ������. ������� �� �������� ������ ����. ��������, ��� 4/3 == 800/600 == 1280/960
		min_distance,       // ������� ��������� ���������. ������ ���� ������ 0.
		max_distance        // ������� ��������� ���������.
	);

	glm::mat4 m_camera_matrix = glm::lookAt(
		m_camera_position, // ������� ������ � ������� ������������
		m_camera_target,   // ��������� ���� �� �������� � ������� ������������
		m_up_vector        // ������, ����������� ����������� �����. ������ (0, 1, 0)
	);
	glm::mat4 m_view_projection_matrix = m_projection_matrix * m_camera_matrix;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CameraComponent, m_camera_position, m_camera_target, m_up_vector, m_camera_matrix, m_projection_matrix, m_view_projection_matrix)
};

glm::vec3 get_world_point_by_screen(entt::registry& registry, float normalized_screen_x, float normalized_screen_y, double depth);

} // namespace diffusion {