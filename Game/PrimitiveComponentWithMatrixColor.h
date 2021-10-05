#pragma once

#include "IGameComponent.h"
#include "Engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <BoundingSphere.h>

#if 0
struct PrimitiveColoredVertex
{
    float x, y, z;
    float color[4];
};
#endif

class PrimitiveComponentWithMatrixColor : public IGameComponent
{
private:
    struct PerSwapchainImageInfo
    {
        vk::CommandBuffer m_command_buffer;

        vk::Fence m_fence;
        vk::Semaphore m_sema;
    };

    vk::PipelineLayout m_layout;
    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;

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

    std::vector<vk::CommandBuffer> m_command_buffers;
    std::vector<PerSwapchainImageInfo> m_infos;

    Game& m_game;


    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    BoundingSphere m_bounding_sphere;

public:
    PrimitiveComponentWithMatrixColor(
        Game& game,
        const std::vector<PrimitiveColoredVertex>& verticies,
        const std::vector<uint32_t>& indexes,
        const BoundingSphere & bounding_sphere, /* TODO: can be generated */
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale,
        const glm::mat4 & CameraMatrix,
        const glm::mat4 & ProjectionMatrix);

    glm::mat4 get_world_matrix();
    void UpdateWorldMatrix(const glm::mat4& world_matrix);
    void UpdateViewMatrix(const glm::mat4& view_matrix);
    void SetWVPMatrix(const glm::mat4 & world_view_projection_matrix);

    vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) override;
    void Initialize(std::size_t num_of_swapchain_images) override;
    void Update(std::size_t num_of_swapchain_images, int width, int height) override;
    void DestroyResources() override;

    bool Intersect(const PrimitiveComponentWithMatrixColor & other);

private:
    void init_cmd_buffer(const vk::CommandBuffer& cmd_buffer, int index);
};

