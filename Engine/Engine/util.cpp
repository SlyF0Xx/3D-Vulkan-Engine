#include "util.h"

#include <vk_mem_alloc.hpp>

buffer_output create_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap)
{
	std::array<uint32_t, 1> queues{ queue };
	auto buffer = game.get_device().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags, vk::SharingMode::eExclusive, queues));
	auto memory_buffer_req = game.get_device().getBufferMemoryRequirements(buffer);

	uint32_t buffer_index = game.find_appropriate_memory_type(memory_buffer_req, game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto memory = game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

	void* mapped_data = nullptr;
	game.get_device().mapMemory(memory, {}, memory_buffer_req.size, {}, &mapped_data);
	std::memcpy(mapped_data, data, buffer_size);

	game.get_device().bindBufferMemory(buffer, memory, {});
	game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(memory, {}, memory_buffer_req.size));

	if (unmap) {
		game.get_device().unmapMemory(memory);
	}

	return { buffer, memory, unmap ? nullptr : static_cast<std::byte*>(mapped_data) };
}

buffer_output sync_create_empty_host_invisible_buffer(Game& game, size_t buffer_size, vk::BufferUsageFlags flags, uint32_t queue)
{
	std::array<uint32_t, 1> queues{ queue };
	auto buffer = game.get_device().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues));
	auto memory_buffer_req = game.get_device().getBufferMemoryRequirements(buffer);

	uint32_t buffer_index = game.find_appropriate_memory_type(memory_buffer_req, game.get_memory_props(), vk::MemoryPropertyFlagBits::eDeviceLocal);

	auto memory = game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));
	game.get_device().bindBufferMemory(buffer, memory, {});

	return { buffer, memory, nullptr };
}

void update_buffer(Game& game, std::size_t buffer_size, void* data, const vk::Buffer& dst_buffer, vk::BufferUsageFlags flags, uint32_t queue)
{
	buffer_output tmp_buffer = create_buffer(game, buffer_size, data, vk::BufferUsageFlagBits::eTransferSrc, queue);

	auto command_buffer = game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];
	std::array regions{ vk::BufferCopy(0, 0, buffer_size) };

	std::array start_barrier{ vk::BufferMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, 0, 0, dst_buffer, 0, buffer_size) };
	std::array finish_barrier{ vk::BufferMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 0, 0, dst_buffer, 0, buffer_size) };

	command_buffer.begin(vk::CommandBufferBeginInfo());
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, start_barrier, {});
	command_buffer.copyBuffer(tmp_buffer.m_buffer, dst_buffer, regions);
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, finish_barrier, {});
	command_buffer.end();


	auto fence = game.get_device().createFence(vk::FenceCreateInfo());

	std::array command_buffers{ command_buffer };
	std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
	game.get_queue().submit(queue_submits, fence);

	game.get_device().waitForFences(fence, VK_TRUE, -1);
	game.get_device().destroyFence(fence);
}

vk::Buffer sync_create_host_invisible_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue)
{
	buffer_output tmp_buffer = create_buffer(game, buffer_size, data, vk::BufferUsageFlagBits::eTransferSrc, queue);


	std::array<uint32_t, 1> queues{ queue };
	auto buffer_memory = game.get_allocator().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues), vma::AllocationCreateInfo(vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly)));


	auto command_buffer = game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];
	std::array regions{ vk::BufferCopy(0, 0, buffer_size) };

	std::array start_barrier{ vk::BufferMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, 0, 0, buffer_memory.first, 0, buffer_size) };
	std::array finish_barrier{ vk::BufferMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 0, 0, buffer_memory.first, 0, buffer_size) };

	command_buffer.begin(vk::CommandBufferBeginInfo());
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, start_barrier, {});
	command_buffer.copyBuffer(tmp_buffer.m_buffer, buffer_memory.first, regions);
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, finish_barrier, {});
	command_buffer.end();



	auto fence = game.get_device().createFence(vk::FenceCreateInfo());

	std::array command_buffers{ command_buffer };
	std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
	game.get_queue().submit(queue_submits, fence);

	game.get_device().waitForFences(fence, VK_TRUE, -1);
	game.get_device().destroyFence(fence);

	return buffer_memory.first;
}
