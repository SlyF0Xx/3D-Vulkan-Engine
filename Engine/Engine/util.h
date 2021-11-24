#pragma once

#include "Engine.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <array>
#include <vector>

struct buffer_output
{
	vk::Buffer m_buffer;
	vma::Allocation m_allocation;
	std::byte* m_mapped_memory;
};

buffer_output create_buffer(Game& game, std::size_t buffer_size, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true);
buffer_output create_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true);

template <typename T>
buffer_output create_buffer(Game& game, const std::vector<T> & data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true)
{
	return create_buffer(game, data.size() * sizeof(T), data.data(), flags, queue, unmap);
}

buffer_output sync_create_empty_host_invisible_buffer(Game& game, size_t buffer_size, vk::BufferUsageFlags flags, uint32_t queue);
void update_buffer(Game& game, std::size_t buffer_size, void* data, const vk::Buffer& dst_buffer, vk::BufferUsageFlags flags, uint32_t queue);

buffer_output sync_create_host_invisible_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue);

template <typename T>
buffer_output sync_create_host_invisible_buffer(Game& game, const std::vector<T>& data, vk::BufferUsageFlags flags, uint32_t queue)
{
	return sync_create_host_invisible_buffer(game, data.size() * sizeof(T), data.data(), flags, queue);
}
