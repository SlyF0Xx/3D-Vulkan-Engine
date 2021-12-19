#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include <nlohmann/json.hpp>

#include <BaseComponents/TagComponent.h>
#include <BaseComponents/DebugComponent.h>
#include <BaseComponents/CameraComponent.h>
#include <BaseComponents/PossessedComponent.h>
#include <BaseComponents/PointLightComponent.h>
#include <BaseComponents/DirectionalLightComponent.h>

#include <Entities/DirectionalLightEntity.h>

namespace Editor {

	class Scene {
	public:
		Scene() = delete;
		explicit Scene(uint32_t id = 0);

		static Scene GetEmpty(uint32_t id = 0);

		void SetData(nlohmann::json& data);

		void Save(std::filesystem::path& source);

		void Load(std::filesystem::path& source);

		std::string GetFileName() const;

		const std::string GetTitle() const;

		// const diffusion::Ref<Game> GetContext() const;
		Game* GetContext() const;

		const uint32_t GetID() const;

	private:
		void CreateEditorCamera();

		void FillBasic();

	private:
		// diffusion::Ref<Game> m_Context;
		Game* m_Context;

		uint32_t m_ID;

		std::string m_Title = "Untitled Scene";

		entt::entity m_EditorCamera;

		nlohmann::json m_SceneData;

		bool m_IsEmpty = true;
		bool m_HasData = false;
	};

}
