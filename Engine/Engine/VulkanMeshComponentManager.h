#pragma once

#include "IMaterial.h"
#include "MeshComponent.h"

#include <unordered_map>

namespace diffusion {

class VulkanMeshComponentManager
{
public:
    const std::unordered_map<int, IMaterial*>& get_materials()
    {
        return m_materials;
    }

    const std::unordered_map<MaterialType, std::unordered_set<int>>& get_materials_by_type()
    {
        return materials_by_type;
    }

    const std::unordered_map<int, std::unordered_set<MeshComponent*>>& get_mesh_by_material()
    {
        return mesh_by_material;
    }

    void register_material(MaterialType material_type, /*std::unique_ptr<*/IMaterial*/*>*/ material);
    void register_mesh(int material_id, /*std::unique_ptr<*/MeshComponent* /*>*/ mesh);

private:
    std::unordered_map<int, /*std::unique_ptr<*/IMaterial*/*>*/> m_materials;
    std::unordered_map<MaterialType, std::unordered_set<int>> materials_by_type;
    std::unordered_map<int, std::unordered_set</*std::unique_ptr<*/MeshComponent*/*>*/>> mesh_by_material;
};

inline VulkanMeshComponentManager s_vulkan_mesh_component_manager;

} // namespace diffusion {