#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

class IMenuRenderer
{
public:
	virtual void Render(const vk::CommandBuffer & command_buffer) = 0;
};