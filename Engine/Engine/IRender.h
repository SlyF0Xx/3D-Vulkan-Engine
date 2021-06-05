#pragma once

#include "export.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

class ENGINE_API IRender
{
public:
    virtual ~IRender() = default;

    virtual void Update() = 0;

    // Only after init objects
    virtual void Initialize() = 0;

    virtual void Draw() = 0;
};