#pragma once

#include "IRender.h"

#include <vk_mem_alloc.hpp>
#include <entt/entt.hpp>

#include <memory>
#include <unordered_set>
#include <unordered_map>

class Game;

class DeferredRender :
    public IRender
{
    struct PerSwapchainImageData
    {
        vk::Image m_deffered_depth_image;
        vma::Allocation m_deffered_depth_memory;

        vk::Image m_albedo_image;
        vma::Allocation m_albedo_memory;

        vk::Image m_normal_image;
        vma::Allocation m_normal_memory;

        vk::ImageView m_deffered_depth_image_view;
        vk::ImageView m_albedo_image_view;
        vk::ImageView m_normal_image_view;

        vk::Sampler m_albedo_sampler;
        vk::Sampler m_normal_sampler;
        vk::ImageView m_deffered_depth_image_view_stencil_only;
        vk::Sampler m_depth_sampler;

        vk::Framebuffer m_deffered_framebuffer;
        vk::Framebuffer m_composite_framebuffer;

        vk::DescriptorSet m_descriptor_set;
        vk::DescriptorSet m_depth_descriptor_set;
    };

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
    entt::registry& m_registry;

    void InitializeDeferredPipelineLayout();
    void InitializeCompositePipelineLayout();
    void InitializeDeferredRenderPass();
    void InitializeCompositeRenderPass();

    void InitializeConstantPerImage();
    void InitializeVariablePerImage();

    void InitializeDeferredPipeline();
    void InitializeCompositePipeline();

    void InitCommandBuffer(int i, const vk::CommandBuffer& command_buffer);

public:
    DeferredRender(Game & game);

    void Update() override;

    // Only after init objects
    void Initialize(int i, const vk::CommandBuffer& command_buffer) override;
};

