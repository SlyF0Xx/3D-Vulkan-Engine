#pragma once

#include "BaseComponents/TagComponent.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/TransformComponent.h"

#include "GameWidget.h"
#include "TagComponentInspector.h"
#include "TransformComponentInspector.h"

#include "SceneInteraction.h"

namespace Editor {

	using namespace diffusion;

	class Inspector : public GameWidget {
	public:
		Inspector() = delete;
		Inspector(const Ref<Game>& game);
		void Render(bool* p_open, ImGuiWindowFlags flags) override;

		void SetDispatcher(const SceneEventDispatcher& dispatcher);

	private:
		void OnEvent(const SceneInteractEvent& sEvent);

	public:
		static inline constexpr const char* TITLE	= "Inspector";
	private:

		bool m_IsSelected							= false;

#pragma region Inspectors
		TagComponentInspector m_TagInspector;
		TransformComponentInspector m_TransformInspector;
#pragma endregion

		SceneEventDispatcher m_SingleDispatcher;

	};

}
