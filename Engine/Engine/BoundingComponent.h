#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "glm_printer.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

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

}

class BoundingComponent :
    public Component
{
public:
    BoundingComponent(
        glm::vec3 center,
        float radius,
        const std::vector<Tag>& tags,
        Entity* parent);

    bool Intersect(const BoundingComponent& other);

    static inline Tag s_bounding_component_tag;

private:
    glm::vec3 m_center;
    float m_radius;
};

} // namespace diffusion {