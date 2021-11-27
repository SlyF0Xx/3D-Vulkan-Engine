#pragma once

#include "../Engine.h"

#include <nlohmann/json.hpp>

namespace diffusion {

struct SubMesh
{
    std::vector<PrimitiveColoredVertex> m_verticies;
    std::vector<uint32_t> m_indexes;

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    } m_bounding_box;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SubMesh, m_verticies, m_indexes)
};

} // namespace diffusion {