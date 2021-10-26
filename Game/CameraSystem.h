#pragma once

#include "CameraComponent.h"

#include "eventpp/callbacklist.h"
#include <glm/glm.hpp>

#include <entt/entt.hpp>

namespace diffusion {

class CameraSystem
{
public:
	CameraSystem(::entt::registry & registry);

	void move_forward(float multiplier);
	void move_backward(float multiplier);
	void move_left(float multiplier);
	void move_right(float multiplier);
	void move_up(float multiplier);
	void move_down(float multiplier);

	eventpp::CallbackList<void(glm::vec3 direction)> callback_list;

private:
	::entt::registry & m_registry;
};

} // namespace diffusion {