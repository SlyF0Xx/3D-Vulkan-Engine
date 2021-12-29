#pragma once

#include "BaseComponents/TagComponent.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/TransformComponent.h"

#include "GameWidget.h"
#include "TagComponentInspector.h"
#include "ScriptComponentInspector.h"
#include "PhysicsComponentInspector.h"
#include "TransformComponentInspector.h"

#include "SceneInteraction.h"
#include "ViewportSnapInteraction.h"
#include "Constants.h"

#include "imgui_lua_bindings.h"

namespace Editor {

	using namespace diffusion;

	struct EditorCreatableComponent {
		enum class EditorCreatableComponentType {
			UNDEFINED,

			TAG,
			TRANSFORM,
			SCRIPT,

			// PHYSICS
			PHYSICS_MASS,
		};

		const char* Title;
		EditorCreatableComponentType Type = EditorCreatableComponentType::UNDEFINED;
		EditorCreatableComponent* Children = nullptr;
		uint32_t Size = 0;

		EditorCreatableComponent(const char* T, EditorCreatableComponentType Ty) : Title(T), Type(Ty), Size(0) {};
		EditorCreatableComponent(const char* T, EditorCreatableComponent* C, uint32_t S) : Title(T), Children(C), Size(S) {};
	};

	class Inspector : public GameWidget {
	public:
		Inspector() = delete;
		Inspector(EDITOR_GAME_TYPE game);
		void Render(bool* p_open, ImGuiWindowFlags flags) override;
		void SetContext(EDITOR_GAME_TYPE ctx) override;
	public:
		static inline constexpr const char* TITLE = "Inspector";

	private:
		void DrawCreatableComponentNode(const EditorCreatableComponent& comp);
	private:

		entt::entity m_Selection = entt::null;

#pragma region Inspectors
		TagComponentInspector m_TagInspector;
		ScriptComponentInspector m_ScriptInspector;
		PhysicsComponentInspector m_PhysicsInspector;
		TransformComponentInspector m_TransformInspector;
#pragma endregion
		SceneEventDispatcher m_SingleDispatcher;

		static inline constexpr const char* POPUP_ADD_COMPONENT = "POPUP_ADD_COMPONENT";

		EditorCreatableComponent m_CreatableComponents[4] = {
			EditorCreatableComponent("Tag Component", EditorCreatableComponent::EditorCreatableComponentType::TAG),
			EditorCreatableComponent("Transform Component", EditorCreatableComponent::EditorCreatableComponentType::TRANSFORM),
			EditorCreatableComponent("Script Component", EditorCreatableComponent::EditorCreatableComponentType::SCRIPT),
			EditorCreatableComponent("Physics Component", new EditorCreatableComponent[1] {
				EditorCreatableComponent("Mass", EditorCreatableComponent::EditorCreatableComponentType::PHYSICS_MASS),
				},
			1),
		};
	};

}
