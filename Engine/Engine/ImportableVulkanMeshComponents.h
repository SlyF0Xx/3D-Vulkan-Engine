#pragma once

#include "Engine.h"
#include "Component.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <entt/entt.hpp>

#include <filesystem>

namespace diffusion {

namespace entt {

void import_mesh(const std::filesystem::path& path, ::entt::registry& registry, ::entt::entity parent_entity);

}

// Edittor component
class ImportableVulkanMeshComponents :
    public Component
{
public:
    ImportableVulkanMeshComponents(
        Game& game,
        const std::filesystem::path& path,
        const std::vector<Tag>& tags,
        Entity* parent);

    inline static Tag s_importable_vulkan_mesh_component_tag;

private:
    int FillMeshes(
        const aiNode& root,
        const aiScene& scene,
        int index,
        int parent_index);

    Game& m_game;
    int m_start_material_index;
};

} // namespace diffusion {