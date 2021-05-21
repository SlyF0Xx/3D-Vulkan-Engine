#pragma once

#include "export.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vector>

struct PerSwapchainImageData
{
    vk::Image m_color_image;
    vk::Image m_depth_image;
    vk::ImageView m_color_image_view;
    vk::ImageView m_depth_image_view;

    vk::DeviceMemory m_depth_memory;
    vk::Framebuffer m_framebuffer;
    vk::Framebuffer m_game_component_framebuffer;

    vk::CommandBuffer m_command_buffer;

    vk::Fence m_fence;
    vk::Semaphore m_sema;
};

class ENGINE_API IGameComponent
{
public:
    virtual ~IGameComponent() = default;
    virtual vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) = 0;
    virtual void Initialize(std::size_t num_of_swapchain_images) = 0;
    virtual void Update(std::size_t num_of_swapchain_images, int width, int height) = 0;
    virtual void DestroyResources() = 0;
};