#include "util.h"

buffer_output create_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue, bool unmap)
{
	std::array<uint32_t, 1> queues{ queue };
	auto buffer_memory =
		game.get_allocator().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags, vk::SharingMode::eExclusive, queues),
			vma::AllocationCreateInfo(vma::AllocationCreateInfo({}, vma::MemoryUsage::eCpuToGpu)));

	void* mapped_data = nullptr;
	game.get_allocator().mapMemory(buffer_memory.second, &mapped_data);
	std::memcpy(mapped_data, data, buffer_size);

	if (unmap) {
		game.get_allocator().unmapMemory(buffer_memory.second);
	}

	return { buffer_memory.first, buffer_memory.second, unmap ? nullptr : static_cast<std::byte*>(mapped_data) };
}

buffer_output sync_create_empty_host_invisible_buffer(Game& game, size_t buffer_size, vk::BufferUsageFlags flags, uint32_t queue)
{
	std::array<uint32_t, 1> queues{ queue };
	auto buffer_memory =
		game.get_allocator().createBuffer(vk::BufferCreateInfo({}, buffer_size, flags | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues),
										  vma::AllocationCreateInfo(vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly)));

	return { buffer_memory.first, buffer_memory.second, nullptr };
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

buffer_output sync_create_host_invisible_buffer(Game& game, std::size_t buffer_size, const void* data, vk::BufferUsageFlags flags, uint32_t queue)
{
	buffer_output tmp_buffer = create_buffer(game, buffer_size, data, vk::BufferUsageFlagBits::eTransferSrc, queue);

	buffer_output out_buffer = sync_create_empty_host_invisible_buffer(game, buffer_size, flags, queue);

	auto command_buffer = game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];
	std::array regions{ vk::BufferCopy(0, 0, buffer_size) };

	std::array start_barrier{ vk::BufferMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, 0, 0, out_buffer.m_buffer, 0, buffer_size) };
	std::array finish_barrier{ vk::BufferMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 0, 0, out_buffer.m_buffer, 0, buffer_size) };

	command_buffer.begin(vk::CommandBufferBeginInfo());
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, start_barrier, {});
	command_buffer.copyBuffer(tmp_buffer.m_buffer, out_buffer.m_buffer, regions);
	command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, finish_barrier, {});
	command_buffer.end();


	auto fence = game.get_device().createFence(vk::FenceCreateInfo());

	std::array command_buffers{ command_buffer };
	std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
	game.get_queue().submit(queue_submits, fence);

	game.get_device().waitForFences(fence, VK_TRUE, -1);
	game.get_device().destroyFence(fence);

	return out_buffer;
}
