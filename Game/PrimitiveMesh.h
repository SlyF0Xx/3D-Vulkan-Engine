#pragma once

#include "IGameComponent.h"
#include "Engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <BoundingSphere.h>

class PrimitiveMesh : public IMesh
{
private:
    // std::vector<vk::DescriptorSet> m_descriptor_sets;
    std::vector<PrimitiveColoredVertex> m_verticies;
    vk::Buffer m_vertex_buffer;
    vk::DeviceMemory m_vertex_memory;
    std::vector<uint32_t> m_indexes;
    vk::Buffer m_index_buffer;
    vk::DeviceMemory m_index_memory;

    glm::mat4 m_world_matrix;
    glm::mat4 m_view_matrix;
    glm::mat4 m_projection_matrix;

    glm::mat4 m_world_view_projection_matrix;
    vk::Buffer m_world_matrix_buffer;
    vk::DeviceMemory m_world_matrix_memory;

    Game& m_game;


    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    BoundingSphere m_bounding_sphere;

public:
    PrimitiveMesh(
        Game& game,
        const std::vector<PrimitiveColoredVertex>& verticies,
        const std::vector<uint32_t>& indexes,
        const BoundingSphere& bounding_sphere, /* TODO: can be generated */
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale,
        const glm::mat4& CameraMatrix,
        const glm::mat4& ProjectionMatrix);

    void InitializeWorldMatrix(
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale);

    void Draw(const vk::CommandBuffer& cmd_buffer) override;

    glm::mat4 get_world_matrix();
    void UpdateWorldMatrix(const glm::mat4& world_matrix);
    void UpdateViewMatrix(const glm::mat4& view_matrix);
    void SetWVPMatrix(const glm::mat4& world_view_projection_matrix);
    bool Intersect(const PrimitiveMesh& other);

    //void DestroyResources() override;


};

