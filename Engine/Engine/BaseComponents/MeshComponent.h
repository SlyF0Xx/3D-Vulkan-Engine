#pragma once

#include "../Engine.h"

#include "TransformComponent.h"

#include <nlohmann/json.hpp>

namespace diffusion {

struct SubMesh
{
    std::vector<PrimitiveColoredVertex> m_verticies;
    std::vector<uint32_t> m_indexes;

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AABB, min, max)
    } m_bounding_box;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SubMesh, m_verticies, m_indexes, m_bounding_box)
};

SubMesh::AABB calculate_bounding_box_in_world_space(entt::registry& registry, const SubMesh& mesh, const TransformComponent& transform);
bool is_in_bounding_box(const SubMesh::AABB & bounding_box, const glm::vec3 & point);

} // namespace diffusion {