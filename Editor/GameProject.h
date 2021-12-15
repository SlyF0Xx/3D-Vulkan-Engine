#pragma once

#include <vector>
#include <filesystem>

#include <Engine.h>
#include <Core/Base.h>

#include "Scene.h"

namespace Editor {

	class GameProject {
	public:
		GameProject() = delete;
		explicit GameProject(const diffusion::Ref<Game>& game);
		bool IsReady() const;

		void CreateEmpty();

		const std::string GetTitle() const;

		const Scene* GetActiveScene() const;

		const std::vector<Scene> GetScenes() const;

	private:
		diffusion::Ref<Game> m_Context;

		std::string m_Title = "Untitled Project";

		std::vector<Scene> m_Scenes;
		uint32_t m_ActiveSceneID;

		std::filesystem::path m_SourcePath;
		bool m_IsReady = false;

	};

}
