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

struct VulkanPointLightCamera
{
    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;
    vk::Buffer m_world_view_projection_matrix_buffer;
    vma::Allocation m_world_view_projection_matrix_memory;
    std::byte* m_world_view_projection_mapped_memory;
};

struct LightShaderInfo
{
    glm::vec3 m_direction;
    float padding = 0;
    glm::mat4 m_ViewProjection;
};

struct PointLightInfo
{
    glm::vec3 m_position;
};

struct VulkanDirectionalLights
{
    struct PipelineInfo
    {
        std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
        vk::PipelineLayout m_layout;

        vk::ShaderModule m_vertex_shader;
        vk::PipelineCache m_cache;
        vk::Pipeline m_pipeline;
    };
    
    struct SwaphainInfo
    {
        vk::Image m_depth_image;
        vma::Allocation m_depth_memory;
        vk::ImageView m_depth_image_view;
        vk::Sampler m_depth_sampler;
    };

    struct PerSwapchainImageData
    {
        SwaphainInfo m_directional_light_info;
        SwaphainInfo m_point_light_info;
    };

    std::vector<PerSwapchainImageData> m_swapchain_data;

    vk::RenderPass m_render_pass;
    PipelineInfo m_directional_light_pipeline;
    PipelineInfo m_point_light_pipeline;

    std::vector<entt::entity> m_directional_light_entities;
    std::vector<entt::entity> m_point_light_entities;

    vk::DescriptorSet m_lights_descriptor_set;
    vk::Buffer m_lights_buffer;
    vma::Allocation m_lights_memory;
};

} // namespace diffusion {