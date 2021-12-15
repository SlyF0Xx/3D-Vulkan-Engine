#pragma once

#include <Engine.h>
#include <Core/Base.h>

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
		explicit Scene(const diffusion::Ref<Game>& game, uint32_t id = 0);

		static Scene GetEmpty(const diffusion::Ref<Game>& game, uint32_t id = 0);

		const std::string GetTitle() const;

		const uint32_t GetID() const;

	private:
		void CreateEditorCamera();

	private:
		diffusion::Ref<Game> m_Context;

		uint32_t m_ID;

		std::string m_Title = "Untitled Scene";

		entt::entity m_EditorCamera;
	};

}
