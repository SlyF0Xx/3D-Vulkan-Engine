#pragma once

#include "IGameComponent.h"
#include "Engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

struct PrimitiveVertex
{
    float x, y, z;
};

class PrimitiveComponent : public IGameComponent
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
    std::vector<PrimitiveVertex> m_verticies;
    vk::Buffer m_vertex_buffer;
    vk::DeviceMemory m_vertex_memory;

    std::vector<vk::CommandBuffer> m_command_buffers;
    std::vector<PerSwapchainImageInfo> m_infos;

    Game& m_game;

public:
    PrimitiveComponent(Game& game, const std::vector<PrimitiveVertex> & verticies);

    vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) override;
    void Initialize(std::size_t num_of_swapchain_images) override;
    void Update(std::size_t num_of_swapchain_images, int width, int height) override;
    void DestroyResources() override;

private:
    void init_cmd_buffer(const vk::CommandBuffer & cmd_buffer, int index);
};
