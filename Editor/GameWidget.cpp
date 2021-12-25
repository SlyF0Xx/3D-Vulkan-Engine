#include "GameWidget.h"

Editor::GameWidget::GameWidget(Game* ctx) {
	SetContext(ctx);
}

void Editor::GameWidget::SetContext(Game* game) {
	m_Context = game;
	Reset();
	InitContexed();
}

void Editor::GameWidget::OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) {
	// ..
}

ImTextureID Editor::GameWidget::GenerateTextureID(EDITOR_GAME_TYPE ctx, diffusion::ImageData& imData, const std::filesystem::path& path) {
	auto command_buffer = ctx->get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(ctx->get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

	command_buffer.begin(vk::CommandBufferBeginInfo());
	imData = ctx->get_texture(command_buffer, path);

	vk::ImageView color_image_view = ctx->get_device().createImageView(
		vk::ImageViewCreateInfo({}, imData.m_image, vk::ImageViewType::e2D, ctx->get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
	);

	vk::Sampler color_sampler = ctx->get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
	command_buffer.end();

	auto fence = ctx->get_device().createFence(vk::FenceCreateInfo());

	std::array command_buffers {command_buffer};
	std::array queue_submits {vk::SubmitInfo({}, {}, command_buffers, {})};
	ctx->get_queue().submit(queue_submits, fence);


	ctx->get_device().waitForFences(fence, VK_TRUE, -1);
	ctx->get_device().destroyFence(fence);

	return ImGui_ImplVulkan_AddTexture(color_sampler, color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
}
