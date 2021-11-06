#include "ContentBrowser.h"

namespace Editor {

	extern const std::filesystem::path g_AssetPath = ".";

	ContentBrowser::ContentBrowser(const diffusion::Ref<Game>& game) : GameWidget(game) {
		m_CurrentDirectory = ".";
	};

	void ContentBrowser::Render(bool* p_open, ImGuiWindowFlags flags) {
		ImGui::Begin(TITLE, p_open, flags);

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath)) {
			if (ImGui::Button("<-")) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 78.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int) (panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (m_TexFile == nullptr || m_TexFolder == nullptr) {
				ImGui::Button(directoryEntry.is_directory() ? "DIR" : "FILE", ImVec2(thumbnailSize, thumbnailSize));
			} else {
				int frame_padding = -1;									// -1 == uses default padding (style.FramePadding)
				ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);     // Size of the image we want to make visible
				ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
				ImVec2 uv1 = ImVec2(1, 1);								// UV coordinates for (thumbnailSize, thumbnailSize) in our texture
				ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);         // Background.
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
				ImGui::ImageButton(directoryEntry.is_directory() ? m_TexFolder : m_TexFile, size, uv0, uv1, frame_padding, bg_col, tint_col);
			}

			if (ImGui::BeginDragDropSource()) {
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();

			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 36, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}

	void ContentBrowser::InitContexed() {
		GameWidget::InitContexed();

		m_TexFile = GenerateTextureID(m_Context, m_TexFileData, FILE_ICON_PATH);
		//ImGuiIO& io = ImGui::GetIO();
		//m_TexFile = io.Fonts->TexID;
		m_TexFolder = GenerateTextureID(m_Context, m_TexFolderData, FOLDER_ICON_PATH);
	}

	ImTextureID ContentBrowser::GenerateTextureID(diffusion::Ref<Game>& ctx, diffusion::ImageData& imData, const std::filesystem::path& path) {
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

}