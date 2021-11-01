#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <array>

class IRender
{
public:
    virtual ~IRender() = default;

    virtual void Update() = 0;
    //virtual void RecreateCommandBuffers() = 0;

    // Only after init objects
    virtual void Initialize(int i, const vk::CommandBuffer& command_buffer) = 0;
};