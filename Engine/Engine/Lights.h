#pragma once

#include "ShadowMap.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

struct LightShaderInfo
{
    LightShaderInfo(
        glm::vec3 direction,
        glm::mat4 ViewProjection)
        : m_direction(direction), m_ViewProjection(ViewProjection)
    {}

    glm::vec3 m_direction;
    uint32_t padding = 0;
    glm::mat4 m_ViewProjection;
};

class Lights
{
private:
    struct PerSwapchainImageData
    {
        vk::Image m_depth_image;
        vk::DeviceMemory m_depth_memory;

        vk::ImageView m_depth_image_view;
        vk::Sampler m_depth_sampler;
    };

    std::vector<PerSwapchainImageData> m_swapchain_data;
    std::vector<ShadowMap> m_lights;

    glm::mat4 m_ProjectionMatrix;

    Game& m_game;

public:
    Lights(
        Game& game,
        const glm::mat4& ProjectionMatrix,
        const std::vector<vk::Image>& swapchain_images);

    void add_light(
        const glm::vec3& position,
        const glm::vec3& cameraTarget,
        const glm::vec3& upVector);

    std::vector<ShadowMap>& get_shadowed_light()
    {
        return m_lights;
    }

    const vk::Sampler& get_depth_sampler(int index)
    {
        return m_swapchain_data[index].m_depth_sampler;
    }

    const vk::ImageView& get_depth_image_view(int index)
    {
        return m_swapchain_data[index].m_depth_image_view;
    }

    std::vector<LightShaderInfo> get_light_shader_info();
};

