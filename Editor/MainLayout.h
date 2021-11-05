#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "imgui_internal.h"

#include <vector>

#pragma region Widgets
#include "ContentBrowser.h"
#include "LuaConsole.h"
#include "SceneHierarchy.h"
#include "widgets/TextEditor.h"
#pragma endregion

#include "FontUtils.h"
#include "EditorLayout.h"
#include "MainWindow.h"

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
		MainLayout(diffusion::Ref<Game>& vulkan) {
			m_ContentBrowser = Editor::ContentBrowser();

			m_TextEditor = TextEditor();
			m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			m_TextEditor.SetTabSize(4);
			m_TextEditor.SetPalette(m_TextEditor.GetLightPalette());
			m_TextEditor.SetShowWhitespaces(false);

			m_LuaConsole = LuaConsole();

			m_SceneHierarchy = SceneHierarchy(vulkan);
		}

		RENDER_STATUS Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void InitDockspace();
		ImVec2 GetSceneSize() const;

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

		ImVec2 m_SceneSize;
		std::vector<ImTextureID> m_TexIDs;
		
	};

}