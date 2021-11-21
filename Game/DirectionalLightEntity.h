#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace diffusion {

::entt::entity create_directional_light_entity(::entt::registry& registry, const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector);

::entt::entity create_point_light_entity(::entt::registry& registry, const glm::vec3& position);

} // namespace diffusion {