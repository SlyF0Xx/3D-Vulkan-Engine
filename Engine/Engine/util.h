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

template <typename T>
buffer_output create_buffer(Game& game, const std::vector<T> & data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap = true)
{
	std::array<uint32_t, 1> queues{ queue };
	std::size_t buffer_size = data.size() * sizeof(T);
	auto buffer = game.get_device().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags, vk::SharingMode::eExclusive, queues));
	auto memory_buffer_req = game.get_device().getBufferMemoryRequirements(buffer);

	uint32_t buffer_index = game.find_appropriate_memory_type(memory_buffer_req, game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto memory = game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

	void* mapped_data = nullptr;
	game.get_device().mapMemory(memory, {}, memory_buffer_req.size, {}, &mapped_data);
	std::memcpy(mapped_data, data.data(), buffer_size);

	game.get_device().bindBufferMemory(buffer, memory, {});
	game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(memory, {}, memory_buffer_req.size));

	if (unmap) {
		game.get_device().unmapMemory(memory);
	}

	return { buffer, memory, unmap ? nullptr : static_cast<std::byte*>(mapped_data) };
}
