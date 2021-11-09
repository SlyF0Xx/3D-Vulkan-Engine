#pragma once

#include "TagComponent.h"
#include "CameraComponent.h"
#include "TransformComponent.h"

#include "GameWidget.h"

#include "SceneInteraction.h"

namespace Editor {

	using namespace diffusion;

	class Inspector : public GameWidget {
	public:
		Inspector() = delete;
		Inspector(const Ref<Game>& game);
		void Render(bool* p_open, ImGuiWindowFlags flags) override;

		void SetDispatcher(const SceneEventDispatcher& dispatcher);

	public:
		static inline constexpr const char* TITLE	= "Inspector";
	private:

		ENTT_ID_TYPE m_SelectionContext				= ENTT_ID_TYPE(-1);
		bool m_IsSelected							= false;

		SceneEventDispatcher m_SingleDispatcher;

	};

}
