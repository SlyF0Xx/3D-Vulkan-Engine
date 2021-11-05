#pragma once

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "Engine.h"
#include "IMenuRenderer.h"

namespace Editor {

	class MenuRenderer : public IMenuRenderer {
	public:
		MenuRenderer(Game& vulkan, ImGui_ImplVulkanH_Window* wd)
			: m_vulkan(vulkan), m_wd(wd) {}

		void Render(const vk::CommandBuffer& command_buffer) {
			ImDrawData* draw_data = ImGui::GetDrawData();
			if (!draw_data) {
				//not initialized yet
				return;
			}
			ImGui_ImplVulkanH_Frame* fd = &m_wd->Frames[m_vulkan.get_presentation_engine().FrameIndex];
			{
				VkRenderPassBeginInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				info.renderPass = m_wd->RenderPass;
				info.framebuffer = fd->Framebuffer;
				info.renderArea.extent.width = m_wd->Width;
				info.renderArea.extent.height = m_wd->Height;
				info.clearValueCount = 1;
				info.pClearValues = &m_wd->ClearValue;
				vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			}

			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

			// Submit command buffer
			vkCmdEndRenderPass(command_buffer);
		}

	private:
		Game& m_vulkan;
		ImGui_ImplVulkanH_Window* m_wd;
	};

}