#pragma once

#include "Engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <array>
#include <vector>

struct buffer_output
{
	vk::Buffer m_buffer;
	vk::DeviceMemory m_memory;
	std::byte* m_mapped_memory;
};

buffer_output create_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true);

template <typename T>
buffer_output create_buffer(Game& game, const std::vector<T> & data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true)
{
	return create_buffer(game, data.size() * sizeof(T), data.data(), flags, queue, unmap);
}

buffer_output sync_create_empty_host_invisible_buffer(Game& game, size_t buffer_size, vk::BufferUsageFlags flags, uint32_t queue);
void update_buffer(Game& game, std::size_t buffer_size, void* data, const vk::Buffer& dst_buffer, vk::BufferUsageFlags flags, uint32_t queue);

vk::Buffer sync_create_host_invisible_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue);

template <typename T>
vk::Buffer sync_create_host_invisible_buffer(Game& game, const std::vector<T>& data, vk::BufferUsageFlags flags, uint32_t queue)
{
	return sync_create_host_invisible_buffer(game, data.size() * sizeof(T), data.data(), flags, queue);
}
