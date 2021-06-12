#pragma once

#include "Engine.h"
#include <BoundingSphere.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <filesystem>

class ImportableMaterial : public IMaterial
{

};

class GameComponentMesh;

class ImportableMesh :
    public IMesh
{
private:
    // std::vector<vk::DescriptorSet> m_descriptor_sets;
    std::vector<PrimitiveColoredVertex> m_verticies;
    vk::Buffer m_vertex_buffer;
    vk::DeviceMemory m_vertex_memory;
    std::vector<uint32_t> m_indexes;
    vk::Buffer m_index_buffer;
    vk::DeviceMemory m_index_memory;


    Game& m_game;
    GameComponentMesh& m_game_component;

    BoundingSphere m_bounding_sphere;
public:
    ImportableMesh(
        Game& game,
        GameComponentMesh & game_component,
        const std::vector<PrimitiveColoredVertex>& verticies,
        const std::vector<uint32_t>& indexes,
        const BoundingSphere& bounding_sphere);

    void Draw(const vk::CommandBuffer& cmd_buffer) override;

    glm::mat4 get_world_matrix() const;
    bool Intersect(const ImportableMesh& other);

    //void DestroyResources() override;
};