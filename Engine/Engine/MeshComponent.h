#pragma once

#include "Engine.h"
#include "Component.h"

namespace diffusion {

namespace entt {

struct SubMesh
{
    std::vector<PrimitiveColoredVertex> m_verticies;
    std::vector<uint32_t> m_indexes;
};

struct MeshComponent
{
    std::vector<SubMesh> m_submeshes;
};

}

class MeshComponent :
    public Component
{
public:
    MeshComponent(
        Game& game,
        const std::vector<PrimitiveColoredVertex>& verticies,
        const std::vector<uint32_t>& indexes,
        const std::vector<Tag>& tags,
        Entity* parent);

    inline static Tag s_mesh_component_tag;

protected:
    std::vector<PrimitiveColoredVertex> m_verticies;
    std::vector<uint32_t> m_indexes;

    Game& m_game;
};

} // namespace diffusion {