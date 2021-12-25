#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "imgui_internal.h"

#include <map>

#pragma region Widgets
#include "CodeEditor.h"
#include "ContentBrowser.h"
#include "LuaConsole.h"
#include "SceneHierarchy.h"
#include "Inspector.h"
#include "widgets/TextEditor.h"
#include "EditorViewport.h"
#include "SceneActionsWidget.h"
#pragma endregion

#include "FontUtils.h"
#include "EditorLayout.h"
#include "MainWindow.h"

#include "Scene.h"
#include "GameProject.h"

#include "SceneInteraction.h"

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
		bool isLuaConsoleOpen = true;
		bool isSceneHierarchyOpen = true;
		bool isViewportOpen = true;
		bool isInspectorOpen = true;

		std::map<entt::entity, bool> ScriptEditorStates;
	};

	class MainLayout : public EditorLayout {
	public:
		MainLayout();

		LayoutRenderStatus Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) override;
		void InitDockspace();
		virtual void OnContextChanged() override;

		void MakeTabVisible(const char* window_name);
		bool IsTabSelected(ImGuiID id);
	private:
		ImGuiDockNodeFlags m_DockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiWindowFlags m_WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		WindowStates m_WindowStates = {};
		LayoutDock m_DockIDs = {};

#pragma region Context sensitive widgets.
		std::map<entt::entity, CodeEditor*> m_CodeEditors;
		ContentBrowser m_ContentBrowser;
		SceneHierarchy m_SceneHierarchy;
		Inspector m_Inspector;
		EditorViewport m_Viewport;
		LuaConsole m_LuaConsole;
		SceneActionsWidget m_ActionsWidget;
#pragma endregion

		SceneEventDispatcher m_SceneDispatcher;
		bool m_IsDockspaceInitialized = false;
		bool m_IsScriptEditing = false;
		entt::entity m_ScriptEditingEntity = entt::null;
	};

}