#pragma once

#include <filesystem>

#include "GameWidget.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "Constants.h"

namespace Editor {

	class ContentBrowser : public GameWidget {

	public:
		static inline constexpr const char* TITLE = "Content Browser";

		ContentBrowser() = delete;
		ContentBrowser(EDITOR_GAME_TYPE game);

		void Render(bool* p_open, ImGuiWindowFlags flags) override;
		void InitContexed() override;

	private:
		static inline constexpr const char* GRID_ICON_PATH		= "./misc/icons/file_icon.png";
		static inline constexpr const char* FOLDER_ICON_PATH	= "./misc/icons/folder_icon.png";

		std::filesystem::path m_CurrentDirectory;

		diffusion::ImageData m_GridTexData;
		ImTextureID m_GridTex;
		diffusion::ImageData m_TexFolderData;
		ImTextureID m_TexFolder;

	};

}