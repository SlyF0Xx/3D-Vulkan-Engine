#include "VulkanMeshComponentManager.h"

void diffusion::VulkanMeshComponentManager::register_material(MaterialType material_type, IMaterial* material)
{
    m_materials.emplace(material->get_id(), material);
    materials_by_type[material_type].insert(material->get_id());
}

void diffusion::VulkanMeshComponentManager::register_mesh(int material_id, MeshComponent* mesh)
{
    mesh_by_material[material_id].emplace(mesh);
}