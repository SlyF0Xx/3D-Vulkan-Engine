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
#include "SceneInteraction.h"
#include "WindowTitleInteraction.h"

namespace Editor {

	enum class GameProjectRenderStatus {
		SUCCESS, LOAD_PROJECT, DELETE_SCENE
	};

	class GameProject {
	public:
		explicit GameProject();
		
		void CreateContext();

		void CreateEmpty();

		void NewScene();

		void RenameScene();

		Editor::GameProjectRenderStatus Render();

		void Load();

		void Save();

		void SaveAs();

		bool PrepareChangeScene(uint32_t id);

		void ActivatePreparedScene();

		void SetActiveScene(uint32_t id);

		void Refresh();

		void ParseMetaFile();

		void DeleteScene();

		void DeleteSceneConfirm();

		const bool HasSourceRoot() const;

		const std::string GetTitle() const;

		Scene* GetActiveScene() const;

		const std::vector<Scene> GetScenes() const;

		const std::filesystem::path GetMetaPath() const;

		const std::filesystem::path GetSourceRoot() const;

		bool IsRunning() const;

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
		void _SetActiveScene(uint32_t id);

	private:
		std::string m_Title = "Untitled Project";

		std::vector<Scene> m_Scenes;
		uint32_t m_ActiveSceneID;
		uint32_t m_NextActiveSceneID;
		uint32_t m_AutoIncrement = 0;

		std::filesystem::path m_SourceDirectoryPath;
		std::filesystem::path m_SourceProjectFilePath;

#pragma region Render State Variables.
		bool m_IsPrepareWindowRequired = false;
		bool m_IsSavingAs = false;
		bool m_IsRenamingScene = false;
		bool m_IsDeletingScene = false;
		bool m_IsSourceFolderChoosing = false;
		bool m_IsProjectFileChoosing = false;
#pragma endregion 

		WindowTitleDispatcher m_WindowTitleDispatcher;
		SceneEventDispatcher m_SceneDispatcher;

		static inline diffusion::Ref<GameProject> s_GameProject;
	};

}
