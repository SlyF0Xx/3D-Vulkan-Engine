#pragma once

#include <string>

#include "GameWidget.h"

#include "Relation.h"
#include "TagComponent.h"

#include "SceneInteraction.h"
#include "Constants.h"

namespace Editor {

	using namespace diffusion;

	class SceneHierarchy : GameWidget {
	public:
		SceneHierarchy() = delete;
		SceneHierarchy(const Ref<Game>& game)
			: GameWidget(game) {};
		/*SceneHierarchy(const Ref<Game>& game, const SceneEventDispatcher& dispatcher) 
			: GameWidget(game), m_SingleDispatcher(dispatcher) {};*/

		void Render(bool* p_open, ImGuiWindowFlags flags) override;

		void DrawEntityNode(ENTT_ID_TYPE entity);

		void SetDispatcher(const SceneEventDispatcher& dispatcher) {
			m_SingleDispatcher = dispatcher;
		}

	private:
		void StartRenaming(const ENTT_ID_TYPE& entity, const char* tag);
		void StopRenaming();

		void SelectOneNotify(const ENTT_ID_TYPE& entity);
		void ResetSelectionNotify();

	public:
		static inline constexpr const char* TITLE = "Hierarchy";

	private:
		// Only for single entities.
		SceneEventDispatcher m_SingleDispatcher;
		
		ENTT_ID_TYPE m_SelectionContext;

		bool m_IsRenameInputFocused = false;
		bool m_IsRenaming = false;
		char m_RenameBuf[Constants::ACTOR_NAME_LENGTH] = "";
	};

}