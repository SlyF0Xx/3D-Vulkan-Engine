#pragma once

#include "export.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

class ENGINE_API IMesh
{
public:
    virtual ~IMesh() = default;
    virtual void Draw(const vk::PipelineLayout & layout, const vk::CommandBuffer& cmd_buffer) = 0;
};
