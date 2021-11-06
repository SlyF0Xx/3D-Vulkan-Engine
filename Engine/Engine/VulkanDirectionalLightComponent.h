#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

namespace diffusion {

struct VulkanDirectionalLightComponent
{
    struct PerSwapchainImageData
    {
        vk::ImageView m_depth_image_view;
        vk::Framebuffer m_framebuffer;
    };

    std::vector<PerSwapchainImageData> m_swapchain_data;
};

struct LightShaderInfo
{
    glm::vec3 m_direction;
    float padding = 0;
    glm::mat4 m_ViewProjection;
};

struct VulkanDirectionalLights
{
    struct PerSwapchainImageData
    {
        vk::Image m_depth_image;
        vma::Allocation m_depth_memory;

        vk::ImageView m_depth_image_view;
        vk::Sampler m_depth_sampler;
    };
    std::vector<PerSwapchainImageData> m_swapchain_data;

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    vk::PipelineLayout m_layout;

    vk::RenderPass m_render_pass;

    vk::ShaderModule m_vertex_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;

    std::vector<entt::entity> m_light_entities;

    vk::DescriptorSet m_lights_descriptor_set;
    vk::Buffer m_lights_buffer;
    vma::Allocation m_lights_memory;
    /*
    vk::Buffer m_lights_count_buffer;
    vma::Allocation m_lights_count_memory;
    */
};

} // namespace diffusion {