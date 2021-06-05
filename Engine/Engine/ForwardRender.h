#pragma once

#include "export.h"

#include "IRender.h"

class ENGINE_API ForwardRender :
    public IRender
{
    struct PerSwapchainImageData
    {
        vk::Image m_color_image;

        vk::Image m_depth_image;
        vk::DeviceMemory m_depth_memory;

        vk::ImageView m_color_image_view;
        vk::ImageView m_depth_image_view;

        vk::Framebuffer m_framebuffer;

        vk::CommandBuffer m_command_buffer;

        vk::Fence m_fence;
        vk::Semaphore m_sema;
    };

    // material
    vk::Format m_color_format = vk::Format::eB8G8R8A8Unorm;
    vk::Format m_depth_format = vk::Format::eD32SfloatS8Uint;
    vk::PhysicalDeviceMemoryProperties m_memory_props;


    vk::Semaphore m_sema;

    std::vector<PerSwapchainImageData> m_swapchain_data;


    vk::RenderPass m_render_pass;
    vk::PipelineLayout m_layout;
    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;

    std::array<vk::DescriptorSetLayout, 1> m_descriptor_set_layouts;
};

