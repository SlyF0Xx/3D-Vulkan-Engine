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
			if (m_GridTex == nullptr || m_TexFolder == nullptr) {
				ImGui::Button(directoryEntry.is_directory() ? "DIR" : "FILE", ImVec2(thumbnailSize, thumbnailSize));
			} else {
				int frame_padding = -1;									// -1 == uses default padding (style.FramePadding)
				ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);     // Size of the image we want to make visible
				ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
				ImVec2 uv1 = ImVec2(1, 1);								// UV coordinates for (thumbnailSize, thumbnailSize) in our texture
				ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);         // Background.
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
				ImGui::ImageButton(directoryEntry.is_directory() ? m_TexFolder : m_GridTex, size, uv0, uv1, frame_padding, bg_col, tint_col);
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

		m_GridTex = GenerateTextureID(m_Context, m_GridTexData, GRID_ICON_PATH);
		m_TexFolder = GenerateTextureID(m_Context, m_TexFolderData, FOLDER_ICON_PATH);
	}

}