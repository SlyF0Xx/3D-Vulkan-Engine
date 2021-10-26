#pragma once

#include "TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

namespace diffusion {

struct RotateTag
{
    ::entt::entity m_entity;
};

class RotateSystem
{
public:
    RotateSystem(::entt::registry& registry);

    void tick();

private:
    ::entt::registry& m_registry;
    glm::mat4 rotation_matrix{ 1 };
    glm::vec3 RotationZ{ 0, 0, 1.0 };
};

} // namespace diffusion {