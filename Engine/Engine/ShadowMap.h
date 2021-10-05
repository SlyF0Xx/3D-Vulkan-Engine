#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

class Game;

class ShadowMap
{
    struct PerSwapchainImageData
    {
        vk::ImageView m_depth_image_view;
        vk::Framebuffer m_framebuffer;
    };

    vk::RenderPass m_render_pass;

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    vk::PipelineLayout m_layout;

    vk::DescriptorSet m_descriptor_set;
    glm::mat4 m_view_projection_matrix;

    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;

    std::vector<PerSwapchainImageData> m_swapchain_data;

    glm::vec3 m_position;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_upVector;
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_CameraMatrix;

    vk::Buffer m_world_view_projection_matrix_buffer;
    vk::DeviceMemory m_world_view_projection_matrix_memory;
    std::byte* m_world_view_projection_mapped_memory;

    Game& m_game;


    void InitializeCompositePipelineLayout();
    void InitializeDescriptorSet();
    void InitializeDeferredRenderPass();

    void DestroyVariablePerImage();
    void InitializeVariablePerImage(int light_index, const std::vector<vk::Image>& swapchain_images);
    
    void InitializeDeferredPipeline();

public:
    ShadowMap(
        Game& game,
        const glm::vec3& position,
        const glm::vec3& cameraTarget,
        const glm::vec3& upVector,
        const glm::mat4& ProjectionMatrix,
        int light_index,
        const std::vector<vk::Image>& swapchain_images);

    void InitCommandBuffer(int index, const vk::CommandBuffer& command_buffer);
    void ResetImages(int light_index, const std::vector<vk::Image>& swapchain_images);

    glm::vec3 get_direction();
    glm::mat4 get_view_proj_matrix();
};

