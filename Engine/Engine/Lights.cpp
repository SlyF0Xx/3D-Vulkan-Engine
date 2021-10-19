#include "Lights.h"

#include "Engine.h"

Lights::Lights(
	Game& game,
	const glm::mat4& ProjectionMatrix,
	const std::vector<vk::Image>& swapchain_images)
	: m_game(game), m_ProjectionMatrix(ProjectionMatrix)
{
	m_swapchain_data.resize(swapchain_images.size());

	std::array<uint32_t, 1> queues{ 0 };
	for (int i = 0; i < m_swapchain_data.size(); ++i) {
		m_swapchain_data[i].m_depth_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, m_lights.size() + 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined));
		m_game.create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

		m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2DArray, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, m_lights.size() + 1)));
		m_swapchain_data[i].m_depth_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
	}
}

void Lights::add_light(const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector)
{
	std::array<uint32_t, 1> queues{ 0 };
	std::vector<vk::Image> images;
	for (int i = 0; i < m_swapchain_data.size(); ++i) {
		m_game.get_device().destroySampler(m_swapchain_data[i].m_depth_sampler);
		m_game.get_device().destroyImageView(m_swapchain_data[i].m_depth_image_view);
		m_game.get_device().destroyImage(m_swapchain_data[i].m_depth_image);
		m_game.get_device().freeMemory(m_swapchain_data[i].m_depth_memory);

		m_swapchain_data[i].m_depth_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, m_lights.size() + 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined));
		m_game.create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

		m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2DArray, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, m_lights.size() + 1)));
		m_swapchain_data[i].m_depth_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

		images.push_back(m_swapchain_data[i].m_depth_image);
	}

	for (int i = 0; i < m_lights.size(); ++i) {
		m_lights[i].ResetImages(i, images);
	}

	m_lights.push_back(ShadowMap(m_game, position, cameraTarget, upVector, m_ProjectionMatrix, m_lights.size(), images));
}

std::vector<LightShaderInfo> Lights::get_light_shader_info()
{
	std::vector<LightShaderInfo> lights;
	for (auto& light : m_lights) {
		lights.push_back(LightShaderInfo{ light.get_direction(), light.get_view_proj_matrix() });
	}

	return lights;
}
