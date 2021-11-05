#include "ImGUIBasedPresentationEngine.h"

namespace Editor {

void ImGUIBasedPresentationEngine::resize(int width, int height)
{
	m_game.get_device().waitIdle();

	std::vector<vk::CommandBuffer> freed_command_buffers;
	for (int i = 0; i < presentation_engine.m_image_count; ++i) {
		m_game.get_device().destroyImageView(presentation_engine.m_swapchain_data[i].m_color_image_view);
		m_game.get_device().destroyImageView(presentation_engine.m_swapchain_data[i].m_depth_image_view);
		m_game.get_allocator().destroyImage(presentation_engine.m_swapchain_data[i].m_depth_image, presentation_engine.m_swapchain_data[i].m_depth_memory);
		m_game.get_allocator().destroyImage(presentation_engine.m_swapchain_data[i].m_color_image, m_color_allocation[i]);
		freed_command_buffers.push_back(presentation_engine.m_swapchain_data[i].m_command_buffer);
	}
	m_color_allocation.clear();
	m_game.get_device().freeCommandBuffers(m_game.get_command_pool(), freed_command_buffers);

	generate_presentation_engine_from_imgui(width, height);
}

void ImGUIBasedPresentationEngine::generate_presentation_engine_from_imgui(int width, int height)
{
	//int width = width <= 0 ? m_Width : width;
	//int height = height <= 0 ? m_Height : height;

	//presentation_engine.m_width = wd->Width;
	//presentation_engine.m_height = wd->Height;

	presentation_engine.m_width = width;
	presentation_engine.m_height = height;
	presentation_engine.use_frame_index_to_render = false;
	presentation_engine.m_surface = m_wd->Surface;
	presentation_engine.m_swapchain = m_wd->Swapchain;
	//presentation_engine.m_color_format = vk::Format(wd->SurfaceFormat.format);

	presentation_engine.m_color_format = vk::Format(m_wd->SurfaceFormat.format);
	presentation_engine.m_depth_format = vk::Format::eD32SfloatS8Uint;
	presentation_engine.m_presentation_mode = vk::PresentModeKHR(m_wd->PresentMode);
	presentation_engine.m_image_count = m_wd->ImageCount;

	presentation_engine.m_final_layout = vk::ImageLayout::eGeneral;


	std::array<uint32_t, 1> queues{ 0 };

	auto command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, presentation_engine.m_image_count));
	presentation_engine.m_swapchain_data.resize(presentation_engine.m_image_count);
	m_color_allocation.resize(presentation_engine.m_image_count);
	for (int i = 0; i < presentation_engine.m_image_count; ++i) {
		//presentation_engine.m_swapchain_data[i].m_color_image = wd->Frames[i].Backbuffer;
		//presentation_engine.m_swapchain_data[i].m_color_image_view = wd->Frames[i].BackbufferView;
		auto color_allocation = m_game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, presentation_engine.m_color_format, vk::Extent3D(width, height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		presentation_engine.m_swapchain_data[i].m_color_image = color_allocation.first;
		m_color_allocation[i] = color_allocation.second;
		presentation_engine.m_swapchain_data[i].m_color_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, presentation_engine.m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));

		//presentation_engine.m_swapchain_data[i].m_command_buffer = wd->Frames[i].CommandBuffer;
		presentation_engine.m_swapchain_data[i].m_command_buffer = command_buffers[i];
		presentation_engine.m_swapchain_data[i].m_fence = m_wd->Frames[i].Fence;

		auto depth_allocation = m_game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, presentation_engine.m_depth_format, vk::Extent3D(presentation_engine.m_width, presentation_engine.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		presentation_engine.m_swapchain_data[i].m_depth_image = depth_allocation.first;
		presentation_engine.m_swapchain_data[i].m_depth_memory = depth_allocation.second;
		presentation_engine.m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, presentation_engine.m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
	}

	presentation_engine.m_sema_data.resize(presentation_engine.m_image_count);
	for (int i = 0; i < presentation_engine.m_image_count; ++i) {
		presentation_engine.m_sema_data[i].m_image_acquired_sema = m_wd->FrameSemaphores[i].ImageAcquiredSemaphore;
		presentation_engine.m_sema_data[i].m_render_complete_sema = m_wd->FrameSemaphores[i].RenderCompleteSemaphore;
	}
}

} // namespace Editor {