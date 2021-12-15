#pragma once

#include "EditorWindow.h"

#include <ImGuizmo.h>

namespace Editor {

	class MainWindow : public EditorWindow {
	public:
		MainWindow() = default;
		MainWindow(Ref<EditorLayout>& layout) : EditorWindow(layout) {};

		void StartMainLoop() override;
		std::string GetWindowTitle() const override;

	private:
		void DispatchCameraMovement();
		void DispatchScriptControl();
		void DispatchKeyInputs();

		entt::entity m_camera;
		entt::entity m_edittor_camera;
	};

}