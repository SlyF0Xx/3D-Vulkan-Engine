#pragma once

#include "Component.h"
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
		m_camera_position, // ������� ������ � ������� ������������
		m_camera_target,   // ��������� ���� �� �������� � ������� ������������
		m_up_vector        // ������, ����������� ����������� �����. ������ (0, 1, 0)
	);
	glm::mat4 m_projection_matrix = glm::perspective(
		static_cast<float>(glm::radians(60.0f)),  // ������������ ���� ������ � ��������. ������ ����� 90&deg; (����� �������) � 30&deg; (�����)
		16.0f / 9.0f,                          // ��������� ������. ������� �� �������� ������ ����. ��������, ��� 4/3 == 800/600 == 1280/960
		0.1f,                                  // ������� ��������� ���������. ������ ���� ������ 0.
		100.0f                                 // ������� ��������� ���������.
	);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CameraComponent, m_camera_position, m_camera_target, m_up_vector, m_camera_matrix, m_projection_matrix)
};

}

class CameraComponent :
	public Component
{
public:
	CameraComponent(Game& game, const std::vector<Tag>& tags, Entity* parent);

	void move_forward(float multiplier);
	void move_backward(float multiplier);
	void move_left(float multiplier);
	void move_right(float multiplier);
	void move_up(float multiplier);
	void move_down(float multiplier);

	eventpp::CallbackList<void(glm::vec3 direction)> callback_list;

	const glm::mat4 & get_camera_matrix()
	{
		return m_camera_matrix;
	}

	const glm::mat4 & get_projection_matrix()
	{
		return m_projection_matrix;
	}

	static inline Tag s_camera_component_tag;
	static inline Tag s_main_camera_component_tag;

protected:
	virtual void recalculate_state();

	Game& m_game;
	glm::mat4 m_camera_matrix;
	glm::mat4 m_projection_matrix;
	glm::vec3 m_camera_position{ 0.0f, -10.0f, 0.0f };
	glm::vec3 m_camera_target{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_up_vector{ 0.0f, 0.0f, -1.0f };
};

} // namespace diffusion {