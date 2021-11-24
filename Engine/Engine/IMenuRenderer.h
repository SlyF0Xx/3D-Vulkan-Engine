#pragma once

#include <vulkan/vulkan.hpp>

class IMenuRenderer
{
public:
	virtual void Render(const vk::CommandBuffer & command_buffer) = 0;
};