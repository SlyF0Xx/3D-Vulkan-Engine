#pragma once

#include "PrimitiveEntity.h"
#include "../PhysicsUtils.h"
#include "../Engine.h"

namespace diffusion{

::entt::entity create_cube_entity(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);

::entt::entity create_cube_entity_lit(
    ::entt::registry& registry,
    glm::vec3 translation = { 0, 0, 0 },
    glm::vec3 rotation = { 0, 0, 0 },
    glm::vec3 scale = { 1, 1, 1 },
    ColliderDefinition collider = {});

::entt::entity create_cube_entity_unlit(
    ::entt::registry& registry,
    glm::vec3 translation = { 0, 0, 0 },
    glm::vec3 rotation = { 0, 0, 0 },
    glm::vec3 scale = { 1, 1, 1 },
    ColliderDefinition collider = {});
} // namespace diffusion {