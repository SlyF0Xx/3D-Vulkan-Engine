#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "imgui_internal.h"

#pragma region Widgets
#include "ContentBrowser.h"
#include "LuaConsole.h"
#include "SceneHierarchy.h"
#include "widgets/TextEditor.h"
#pragma endregion

#include "EditorLayout.h"

namespace Editor {

	struct WindowStates {
		bool isContentBrowserOpen = true;
	};

	class MainLayout : public EditorLayout {
	public:
		MainLayout(diffusion::Ref<Game>& vulkan);
		void Render(Game& vulkan, PresentationEngine& engine) override;

	private:
		WindowStates m_WindowStates = {};

		ContentBrowser m_ContentBrowser;
		LuaConsole m_LuaConsole;
		SceneHierarchy m_SceneHierarchy;
		TextEditor m_TextEditor;
		
	};

}