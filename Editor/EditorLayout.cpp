#include "EditorLayout.h"

void Editor::EditorLayout::OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
	engine.resize(m_SceneSize.x, m_SceneSize.y);
	vulkan.InitializePresentationEngine(engine.get_presentation_engine());
	vulkan.SecondInitialize();

	m_TexIDs.clear();
	for (auto& swapchain_data : engine.get_presentation_engine().m_swapchain_data) {
		vk::Sampler color_sampler = vulkan.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}
}

ImVec2 Editor::EditorLayout::GetSceneSize() const {
	return m_SceneSize;
}
