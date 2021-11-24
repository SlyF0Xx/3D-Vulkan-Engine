#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

namespace diffusion {

struct VulkanPointLightComponent
{
    struct PerSwapchainImageData
    {
        vk::ImageView m_depth_image_view;
        vk::Framebuffer m_framebuffer;
    };

    std::vector<PerSwapchainImageData> m_swapchain_data;
};

} // namespace diffusion {