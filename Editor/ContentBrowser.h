#pragma once

#include <filesystem>

#include "GameWidget.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

namespace Editor {

	class ContentBrowser : GameWidget {

	public:
		static inline constexpr const char* TITLE = "Content Browser";

		ContentBrowser() = delete;
		ContentBrowser(const diffusion::Ref<Game>& game);

		void Render(bool* p_open, ImGuiWindowFlags flags) override;
		void InitContexed() override;

	private:

		//void CreateTexture(Game& vulkan, const char* path);

	private:
		static inline constexpr const char* FILE_ICON_PATH		= "./misc/icons/file_icon.png";
		static inline constexpr const char* FOLDER_ICON_PATH	= "./misc/icons/folder_icon.png";

		static ImTextureID GenerateTextureID(diffusion::Ref<Game>& ctx, diffusion::ImageData& imData, const std::filesystem::path& path);

		std::filesystem::path m_CurrentDirectory;

		diffusion::ImageData m_TexFileData;
		ImTextureID m_TexFile;
		diffusion::ImageData m_TexFolderData;
		ImTextureID m_TexFolder;

	};

}