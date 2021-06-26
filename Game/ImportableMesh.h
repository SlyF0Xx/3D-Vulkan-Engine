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

struct ImageData {
    vk::Image m_image;
    vk::DeviceMemory m_memory;
};

class UnlitMaterial : public IMaterial
{
protected:
    vk::Image m_albedo_image;
    vk::DeviceMemory m_albedo_memory;
    vk::ImageView m_albedo_image_view;
    vk::Sampler m_albedo_sampler;

    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    Game& m_game;

    ImageData prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath);

public:
    UnlitMaterial(Game& game);
    UnlitMaterial(Game& game, const std::filesystem::path& texture_path);

    void UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer) override;
};

class ImportableMaterial : public UnlitMaterial
{
private:
    vk::Image m_normal_image;
    vk::DeviceMemory m_normal_memory;
    vk::ImageView m_normal_image_view;
    vk::Sampler m_normal_sampler;

public:
    ImportableMaterial(
        Game& game, const std::filesystem::path & texture_path, const std::filesystem::path& normal_path);
    void UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer) override;
};

class DefaultMaterial : public UnlitMaterial
{
public:
    DefaultMaterial(Game& game);
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

    void Draw(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer) override;

    glm::mat4 get_world_matrix() const;
    bool Intersect(const ImportableMesh& other);

    //void DestroyResources() override;
};