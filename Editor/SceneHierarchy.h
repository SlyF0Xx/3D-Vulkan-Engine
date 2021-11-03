#pragma once

#include "BaseWidget.h"
#include "Engine.h"
#include "Core/Base.h"
#include "TagComponent.h"
#include "CameraComponent.h"

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
	public:
		static inline constexpr const char* TITLE = "Hierarchy";

	private:
		Ref<Game> m_Context;
		ENTT_ID_TYPE m_SelectionContext;
	};

}