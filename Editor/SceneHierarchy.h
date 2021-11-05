#pragma once

#include <string>

#include "BaseWidget.h"
#include "Engine.h"
#include "Core/Base.h"
#include "Relation.h"
#include "TagComponent.h"

namespace Editor {

	using namespace diffusion;

	class SceneHierarchy : Widget {
	public:
		SceneHierarchy() = default;
		SceneHierarchy(const Ref<Game>& game);

		void SetContext(const Ref<Game>& game);

		void Render();

		virtual void Render(bool* p_open, ImGuiWindowFlags flags);

		void DrawEntityNode(ENTT_ID_TYPE entity);

	private:
		void StartRenaming(ENTT_ID_TYPE& entity, const char* tag);
		void StopRenaming();

	public:
		static inline constexpr const char* TITLE = "Hierarchy";

	private:
		static inline constexpr const int RENAME_BUF_SIZE = 64;

		Ref<Game> m_Context;
		ENTT_ID_TYPE m_SelectionContext;

		bool m_IsRenameInputFocused = false;
		bool m_IsRenaming = false;
		char m_RenameBuf[RENAME_BUF_SIZE] = "";
	};

}