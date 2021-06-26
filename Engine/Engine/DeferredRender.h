#pragma once

#include "export.h"

#include "IRender.h"

#include <memory>
#include <unordered_set>
#include <unordered_map>

class Game;

class ENGINE_API DeferredRender :
    public IRender
{
    struct PerSwapchainImageData
    {
        vk::Image m_color_image;

        vk::Image m_deffered_depth_image;
        vk::DeviceMemory m_deffered_depth_memory;

        vk::Image m_depth_image;
        vk::DeviceMemory m_depth_memory;

        vk::Image m_albedo_image;
        vk::DeviceMemory m_albedo_memory;

        vk::Image m_normal_image;
        vk::DeviceMemory m_normal_memory;

        vk::ImageView m_color_image_view;
        vk::ImageView m_deffered_depth_image_view;
        vk::ImageView m_depth_image_view;
        vk::ImageView m_albedo_image_view;
        vk::ImageView m_normal_image_view;

        vk::Sampler m_albedo_sampler;
        vk::Sampler m_normal_sampler;
        vk::ImageView m_deffered_depth_image_view_stencil_only;
        vk::Sampler m_depth_sampler;

        vk::Framebuffer m_deffered_framebuffer;
        vk::Framebuffer m_composite_framebuffer;

        vk::CommandBuffer m_command_buffer;

        vk::Fence m_fence;
        vk::Semaphore m_sema;

        vk::DescriptorSet m_descriptor_set;
        vk::DescriptorSet m_depth_descriptor_set;
    };

    vk::Semaphore m_sema;

    std::vector<PerSwapchainImageData> m_swapchain_data;

    vk::RenderPass m_deffered_render_pass;
    vk::ShaderModule m_deffered_vertex_shader;
    vk::ShaderModule m_deffered_fragment_shader;
    vk::PipelineCache m_deffered_cache;
    vk::Pipeline m_deffered_pipeline;


    vk::RenderPass m_composite_render_pass;
    vk::ShaderModule m_composite_vertex_shader;
    vk::ShaderModule m_composite_fragment_shader;
    vk::PipelineCache m_composite_cache;
    vk::Pipeline m_composite_pipeline;

    vk::PipelineLayout m_deferred_layout;
    vk::PipelineLayout m_composite_layout;
    std::array<vk::DescriptorSetLayout, 2> m_composite_descriptor_set_layouts;

    Game& m_game;

    void InitializeDeferredPipelineLayout();
    void InitializeCompositePipelineLayout();
    void InitializeDeferredRenderPass();
    void InitializeCompositeRenderPass();

    void InitializeConstantPerImage();
    void InitializeVariablePerImage(const std::vector<vk::Image>& swapchain_images);

    void InitializeDeferredPipeline();
    void InitializeCompositePipeline();

    void InitCommandBuffer();

public:
    DeferredRender(Game & game, const std::vector<vk::Image> & swapchain_images);

    void Update(const std::vector<vk::Image>& swapchain_images) override;

    // Only after init objects
    void Initialize() override;

    void Draw() override;
};

