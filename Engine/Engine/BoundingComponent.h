#pragma once
#include "Component.h"

#include <glm/glm.hpp>

namespace diffusion {

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