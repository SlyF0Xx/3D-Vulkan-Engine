#pragma once

#include "TransformComponent.h"
#include "../glm_printer.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

struct BoundingComponent {
    glm::vec3 m_center;
    float m_radius;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BoundingComponent, m_center, m_radius)
};

bool intersect(::entt::registry& registry, const BoundingComponent& lhv, const BoundingComponent& rhv);
bool intersect(::entt::registry& registry,
    const BoundingComponent& lhv,
    const BoundingComponent& rhv,
    const TransformComponent& lhv_transform,
    const TransformComponent& rhv_transform);

} // namespace diffusion {