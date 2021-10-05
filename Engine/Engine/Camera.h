#pragma once

#include "Engine.h"

#include "eventpp/callbacklist.h"
#include <glm/glm.hpp>

namespace diffusion {

class Camera
{
public:
	Camera(Game& game);

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


private:
	void recalculate_state();

	Game& m_game;
	glm::mat4 m_camera_matrix;
	glm::mat4 m_projection_matrix;
	glm::vec3 m_camera_position{ 0.0f, 0.0f, -10.0f };
	glm::vec3 m_camera_target{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_up_vector{ 0.0f, -1.0f, 0.0f };
};

} // namespace diffusion {