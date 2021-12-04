#pragma once

#include "EditorWindow.h"

#include <ImGuizmo.h>

namespace Editor {

	class MainWindow : public EditorWindow {
	public:
		MainWindow() = default;
		MainWindow(Ref<EditorLayout>& layout) : EditorWindow(layout) {};

		void StartMainLoop() override;
		const char* GetWindowTitle() const override;
	};

}