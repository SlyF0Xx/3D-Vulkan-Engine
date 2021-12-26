#pragma once

#include "EditorWindow.h"
#include "SceneInteraction.h"

#include <ImGuizmo.h>

namespace Editor {

	class MainWindow : public EditorWindow {
	public:
		MainWindow();

		void StartMainLoop() override;
		std::string GetWindowTitle() const override;

	protected:

		// void OnContextChanged() override;

	private:
		void DispatchCameraMovement();
		void DispatchScriptControl();
		void DispatchKeyInputs();

		entt::entity m_camera;
		entt::entity m_edittor_camera;

		SceneEventDispatcher m_SceneDispatcher;
	};

}