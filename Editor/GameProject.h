#pragma once

#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include <Engine.h>
#include <Core/Base.h>

#include "Scene.h"
#include "Constants.h"
#include "WindowTitleInteraction.h"

namespace Editor {

	class GameProject {
	public:
		explicit GameProject();
		
		void CreateContext();

		void CreateEmpty();

		void NewScene();

		void RenameScene();

		void Render();

		void Load();

		void Save();

		void SaveAs();

		void SetActiveScene(uint32_t id);

		void Refresh();

		const bool HasSourceRoot() const;

		const std::string GetTitle() const;

		Scene* GetActiveScene() const;

		const std::vector<Scene> GetScenes() const;

		const std::filesystem::path GetMetaPath() const;

		const std::filesystem::path GetSourceRoot() const;

		static inline diffusion::Ref<GameProject> Instance() {
			if (!GameProject::s_GameProject) {
				GameProject::s_GameProject = diffusion::CreateRef<GameProject>();
				return GameProject::Instance();
			}
			return GameProject::s_GameProject;
		}

		// diffusion::Ref<Game> GetCurrentContext() const;
		Game* GetCurrentContext() const;

	private:
		void ParseMetaFile(std::filesystem::path path);

		void _SetActiveScene(uint32_t id);

	private:
		std::string m_Title = "Untitled Project";

		std::vector<Scene> m_Scenes;
		uint32_t m_ActiveSceneID;

		std::filesystem::path m_SourcePath;

		bool m_IsSavingAs = false;
		bool m_IsRenamingScene = false;
		bool m_IsSourceFolderChoosing = false;
		bool m_IsProjectFileChoosing = false;

		WindowTitleDispatcher m_WindowTitleDispatcher;

		static inline diffusion::Ref<GameProject> s_GameProject;
	};

}
