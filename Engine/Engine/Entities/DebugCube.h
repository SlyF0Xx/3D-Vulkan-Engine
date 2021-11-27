#pragma once

#include "PrimitiveEntity.h"
#include "../Engine.h"

namespace diffusion {

::entt::entity create_debug_cube_entity(
    ::entt::registry& registry,
    glm::vec3 translation = { 0, 0, 0 },
    glm::vec3 rotation = { 0, 0, 0 },
    glm::vec3 scale = { 1, 1, 1 });

::entt::entity create_debug_sphere_entity(
    ::entt::registry& registry,
    glm::vec3 translation = { 0, 0, 0 },
    glm::vec3 rotation = { 0, 0, 0 },
    glm::vec3 scale = { 1, 1, 1 });

} // namespace diffusion {