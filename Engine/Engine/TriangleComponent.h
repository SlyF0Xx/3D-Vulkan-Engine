#pragma once

#include "IGameComponent.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#if 0
struct PrimitiveVertex
{
    float x, y, z;
};

class GameImpl;

class TriangleComponent : public IGameComponent
{
private:
    struct PerSwapchainImageInfo
    {
        vk::Image m_depth_image;
        vk::ImageView m_color_image_view;
        vk::ImageView m_depth_image_view;
        vk::Image m_color_image;

        vk::Framebuffer m_framebuffer;
        vk::DeviceMemory m_depth_memory; // maybe remove?
        vk::CommandBuffer m_command_buffer;

        vk::Fence m_fence;
        vk::Semaphore m_sema;
    };

    vk::RenderPass m_render_pass;

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

    GameImpl& m_game;
    vk::Format m_color_format;
    vk::Format m_depth_format;
    int m_width;
    int m_height;
    const vk::PhysicalDeviceMemoryProperties& m_memory_props;

public:
    TriangleComponent(GameImpl& game,
        vk::Format color_format,
        vk::Format depth_format,
        int width,
        int height,
        const vk::PhysicalDeviceMemoryProperties& memory_props);

    vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) override;
    void Initialize(const std::vector<vk::Image>&) override;
    void DestroyResources() override;
};
#endif