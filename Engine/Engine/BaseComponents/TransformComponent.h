#pragma once

#include "../glm_printer.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

struct TransformComponent
{
	glm::mat4 m_world_matrix;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TransformComponent, m_world_matrix)
};

glm::mat4 calculate_global_world_matrix(::entt::registry & registry, const TransformComponent & component);

glm::mat4 create_matrix_by_location_target(
	const glm::vec3& position,
	const glm::vec3& target);

glm::mat4 create_matrix(
	const glm::vec3& position = glm::vec3(0),
	const glm::vec3& rotation = glm::vec3(0),
	const glm::vec3& scale = glm::vec3(1));

} // namespace diffusion {