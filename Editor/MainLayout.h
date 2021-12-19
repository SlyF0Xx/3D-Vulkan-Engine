#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "imgui_internal.h"

#include <vector>

#pragma region Widgets
#include "ContentBrowser.h"
#include "LuaConsole.h"
#include "SceneHierarchy.h"
#include "Inspector.h"
#include "widgets/TextEditor.h"
#include "EditorViewport.h"
#pragma endregion

#include "FontUtils.h"
#include "EditorLayout.h"
#include "MainWindow.h"

#include "Scene.h"
#include "GameProject.h"

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
	};

	class MainLayout : public EditorLayout {
	public:
		MainLayout();

		LayoutRenderStatus Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void InitDockspace();
		virtual void OnContextChanged() override;

	private:
		ImGuiDockNodeFlags m_DockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiWindowFlags m_WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		WindowStates m_WindowStates = {};
		LayoutDock m_DockIDs = {};

#pragma region Context sensitive widgets.
		ContentBrowser m_ContentBrowser;
		SceneHierarchy m_SceneHierarchy;
		Inspector m_Inspector;
		EditorViewport m_Viewport;
#pragma endregion

#pragma region Widgets.
		TextEditor m_TextEditor;
		LuaConsole m_LuaConsole;
#pragma endregion

		bool m_IsDockspaceInitialized = false;
	};

}