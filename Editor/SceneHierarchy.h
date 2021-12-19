#pragma once

#define USE_STD_FILESYSTEM

#include <string>
#include <filesystem>

#include "GameWidget.h"
#include "ImGuiFileDialog.h"

#include "BaseComponents/Relation.h"
#include "BaseComponents/TagComponent.h"
#include "BaseComponents/CameraComponent.h"

#include "Entities/DebugCube.h"
#include "Entities/CubeEntity.h"
#include "Entities/PlaneEntity.h"
#include "Entities/ImportableEntity.h"
#include "Entities/DirectionalLightEntity.h"

#include "SceneInteraction.h"
#include "Constants.h"

namespace Editor {

	using namespace diffusion;

	struct EditorCreatableEntity {
		enum class EditorCreatableEntityType {
			UNDEFINED,

			PRIMITIVE_CUBE,
			PRIMITIVE_PLANE,

			LIGHT_DIRECTIONAL,

			DEBUG_CUBE,

			IMPORTABLE
		};

		const char* Title;
		EditorCreatableEntityType Type		= EditorCreatableEntityType::UNDEFINED;
		EditorCreatableEntity* Children		= nullptr;
		uint32_t Size = 0;

		EditorCreatableEntity(const char* T, EditorCreatableEntityType Ty) : Title(T), Type(Ty), Size(0) {};
		EditorCreatableEntity(const char* T, EditorCreatableEntity* C, uint32_t S) : Title(T), Children(C), Size(S) {};
	};

	class SceneHierarchy : public GameWidget {
	public:
		SceneHierarchy() = delete;
		SceneHierarchy(EDITOR_GAME_TYPE game);

		void Render(bool* p_open, ImGuiWindowFlags flags) override;

		void DrawEntityNode(ENTT_ID_TYPE entity);
		void DrawCreatableEntityNode(EditorCreatableEntity entity);

	private:
		void StartRenaming(const ENTT_ID_TYPE& entity, const char* tag);
		void StopRenaming();

		void SelectOneNotify(const ENTT_ID_TYPE& entity);
		void ResetSelectionNotify();

		void InitContexed() override;

	public:
		static inline constexpr const char* TITLE = "Hierarchy";

	private:
		static inline constexpr const char* POPUP_ADD_ENTITY = "POPUP_ADD_ENTITY";

		EditorCreatableEntity m_CreatableEntities[4] = {
			EditorCreatableEntity("Primitives", 
				new EditorCreatableEntity[2] {
					EditorCreatableEntity("Cube", EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_CUBE),
					EditorCreatableEntity("Plane", EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_PLANE),
				},
			2),
			EditorCreatableEntity("Lights", 
				new EditorCreatableEntity[1] {
					EditorCreatableEntity("Directional Light", EditorCreatableEntity::EditorCreatableEntityType::LIGHT_DIRECTIONAL),
				},
			1),
			EditorCreatableEntity("Debug", new EditorCreatableEntity[1] {
					EditorCreatableEntity("Cube", EditorCreatableEntity::EditorCreatableEntityType::DEBUG_CUBE),
				},
			1),
			EditorCreatableEntity("Import...", EditorCreatableEntity::EditorCreatableEntityType::IMPORTABLE),
		};
		
		SceneEventDispatcher m_SingleDispatcher;
		
		ENTT_ID_TYPE m_SelectionContext;

		bool m_IsRenameInputFocused = false;
		bool m_IsRenaming = false;
		char m_RenameBuf[Constants::ACTOR_NAME_LENGTH] = "";
	};

}