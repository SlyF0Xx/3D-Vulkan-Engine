#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

class IMesh
{
public:
    virtual ~IMesh() = default;
    virtual void Draw(const vk::PipelineLayout & layout, const vk::CommandBuffer& cmd_buffer) = 0;
};
