#pragma once

#include "Engine.h"

#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

struct SubMesh
{
    std::vector<PrimitiveColoredVertex> m_verticies;
    std::vector<uint32_t> m_indexes;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SubMesh, m_verticies, m_indexes)
};

}

} // namespace diffusion {