#include "EditorLayout.h"

void Editor::EditorLayout::OnResize(Game& vulkan, PresentationEngine& engine) {
	// TODO: recalculate size
	/*engine.m_width = m_SceneSize.x;
	engine.m_height = m_SceneSize.y;

	vulkan.InitializePresentationEngine(engine);
	vulkan.SecondInitialize();*/

	/*m_TexIDs.clear();
	for (auto& swapchain_data : engine.m_swapchain_data) {
		vk::Sampler color_sampler = vulkan.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}*/
}

ImVec2 Editor::EditorLayout::GetSceneSize() const {
	return m_SceneSize;
}
