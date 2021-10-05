#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vector>

class IGameComponent
{
public:
    virtual ~IGameComponent() = default;
    virtual vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) = 0;
    virtual void Initialize(std::size_t num_of_swapchain_images) = 0;
    virtual void Update(std::size_t num_of_swapchain_images, int width, int height) = 0;
    virtual void DestroyResources() = 0;
};