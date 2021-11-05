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

#include "FontUtils.h"
#include "EditorLayout.h"

namespace Editor {

	struct LayoutDock {
		ImGuiID MainDock;
		ImGuiID TopDock;
		ImGuiID LeftDock;
		ImGuiID DownDock;
		ImGuiID RightDock;
	};

	struct WindowStates {
		bool isDocksSpaceOpen = true;
		bool isContentBrowserOpen = true;
	};

	class MainLayout : public EditorLayout {
	public:
		MainLayout(diffusion::Ref<Game>& vulkan);
		void Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void InitDockspace();

	private:
		ImGuiDockNodeFlags m_DockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiWindowFlags m_WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		WindowStates m_WindowStates = {};
		LayoutDock m_DockIDs = {};

		ContentBrowser m_ContentBrowser;
		LuaConsole m_LuaConsole;
		SceneHierarchy m_SceneHierarchy;
		TextEditor m_TextEditor;

		bool m_IsDockspaceInitialized = false;
		
	};

}