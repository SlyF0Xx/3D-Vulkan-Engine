#pragma once

#include "export.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

// Actually, DescriptorSet - different textures
class ENGINE_API IMaterial
{
    int id;
public:
    IMaterial();
    virtual ~IMaterial() = default;

    virtual void UpdateMaterial(const vk::PipelineLayout & layout, const vk::CommandBuffer& cmd_buffer) = 0;
    int get_id() { return id; };

    static int get_start_id();
};

enum class ENGINE_API MaterialType
{
    Opaque
};
