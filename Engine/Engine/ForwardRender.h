#pragma once

#include "IRender.h"

#include <vk_mem_alloc.hpp>
#include <entt/entt.hpp>

#include <memory>
#include <unordered_set>
#include <unordered_map>

class Game;

class ForwardRender :
    public IRender
{
    struct PerSwapchainImageData
    {
        // Variable per image value
        vk::Framebuffer m_framebuffer;

        vk::DescriptorSet m_shadows_descriptor_set;
    };

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    vk::PipelineLayout m_layout;

    std::vector<PerSwapchainImageData> m_swapchain_data;

    vk::RenderPass m_render_pass;
    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;



    std::vector<vk::DescriptorSetLayout> m_debug_descriptor_set_layouts;
    vk::PipelineLayout m_debug_layout;

    vk::ShaderModule m_debug_vertex_shader;
    vk::ShaderModule m_debug_fragment_shader;
    vk::PipelineCache m_debug_cache;
    vk::Pipeline m_debug_pipeline;

    Game& m_game;

    void InitializePipelineLayout();

    void InitializeRenderPass();
    void DestroyRenderPass();

    void InitializeConstantPerImage();
    // screen dependent
    // flexiable
    void InitializeVariablePerImage();
    void DestroyVariablePerImageResources();

    void InitializePipeline();
    void InitializeDebugPipeline();
    void DestroyPipeline();

    void InitCommandBuffer(int i, const vk::CommandBuffer& command_buffer);

    void DestroyResources();

public:
    ForwardRender(Game& game);
    ~ForwardRender();

    void Update() override;

    // Only after init objects
    void Initialize(int i, const vk::CommandBuffer& command_buffer) override;
};

