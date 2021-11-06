#pragma once

#include "EditorWindow.h"

namespace Editor {

	class MainWindow : public EditorWindow {
	public:
		MainWindow() = default;
		MainWindow(Ref<EditorLayout>& layout, Ref<Game>& context) : EditorWindow(layout, context) {};

		void StartMainLoop() override;
		const char* GetWindowTitle() const override;
	};

}