#include "MainLayout.h"

Editor::MainLayout::MainLayout() :
	EditorLayout(),
	m_ContentBrowser(m_Context),
	m_SceneHierarchy(m_Context),
	m_Inspector(m_Context),
	m_Viewport(m_Context),
	m_LuaConsole(m_Context),
	m_ActionsWidget(m_Context) {

	m_TextEditor = TextEditor();
	m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	m_TextEditor.SetTabSize(4);
	m_TextEditor.SetPalette(m_TextEditor.GetLightPalette());
	m_TextEditor.SetShowWhitespaces(false);
}

Editor::LayoutRenderStatus Editor::MainLayout::Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
	ImGui::PushStyleColor(ImGuiCol_Button, Constants::SUCCESS_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants::HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, Constants::ACTIVE_COLOR);

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::Begin("DockSpace", &m_WindowStates.isDocksSpaceOpen, m_WindowFlags);
	m_DockIDs.MainDock = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockIDs.MainDock, ImVec2(0.0f, 0.0f), m_DockspaceFlags);

	InitDockspace();
	ImGui::End();

	if (ImGui::BeginMainMenuBar()) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Scene")) {
				Editor::GameProject::Instance()->NewScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Load project")) {
				Editor::GameProject::Instance()->Load();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Rename Scene")) {
				Editor::GameProject::Instance()->RenameScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Project")) {
				Editor::GameProject::Instance()->Save();
			}
			if (ImGui::MenuItem("Save Project as...")) {
				Editor::GameProject::Instance()->SaveAs();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				GetParent()->Destroy();
				return Editor::LayoutRenderStatus::EXIT;
			}

			ImGui::EndMenu();
		}



		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::MenuItem("Content Browser", NULL, m_WindowStates.isContentBrowserOpen)) {
				m_WindowStates.isContentBrowserOpen = !m_WindowStates.isContentBrowserOpen;
			}

			if (ImGui::MenuItem("Lua Console", NULL, m_WindowStates.isLuaConsoleOpen)) {
				m_WindowStates.isLuaConsoleOpen = !m_WindowStates.isLuaConsoleOpen;
			}

			if (ImGui::MenuItem("Inspector", NULL, m_WindowStates.isInspectorOpen)) {
				m_WindowStates.isInspectorOpen = !m_WindowStates.isInspectorOpen;
			}

			if (ImGui::MenuItem("Scene Hierarchy", NULL, m_WindowStates.isSceneHierarchyOpen)) {
				m_WindowStates.isSceneHierarchyOpen = !m_WindowStates.isSceneHierarchyOpen;
			}

			if (ImGui::MenuItem("Viewport", NULL, m_WindowStates.isViewportOpen)) {
				m_WindowStates.isViewportOpen = !m_WindowStates.isViewportOpen;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scenes")) {
			for (const Scene& scene : GameProject::Instance()->GetScenes()) {
				if (ImGui::MenuItem((std::to_string(scene.GetID()) + ") " + scene.GetTitle()).c_str(), NULL,
					scene.GetID() == GameProject::Instance()->GetActiveScene()->GetID())) {
					GameProject::Instance()->SetActiveScene(scene.GetID());
				}
			}

			ImGui::EndMenu();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::EndMainMenuBar();
	}

	if (m_WindowStates.isSceneHierarchyOpen) {
		m_SceneHierarchy.Render(&m_WindowStates.isSceneHierarchyOpen, 0);
	}

	if (m_WindowStates.isLuaConsoleOpen) {
		ImGui::SetNextWindowDockID(m_DockIDs.DownDock, ImGuiCond_Once);
		m_LuaConsole.Render(&m_WindowStates.isLuaConsoleOpen, 0);
	}

	if (m_WindowStates.isContentBrowserOpen) {
		m_ContentBrowser.Render(&m_WindowStates.isContentBrowserOpen, 0);
	}

	ImGui::SetNextWindowDockID(m_DockIDs.MainDock, ImGuiCond_Once);
	ImGui::Begin("Code editor");
	ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY));
	m_TextEditor.Render("Editor", ImVec2(0, 0), true);
	ImGui::PopFont();
	ImGui::End();

	if (m_WindowStates.isViewportOpen) {
		ImGui::SetNextWindowDockID(m_DockIDs.TopDock, ImGuiCond_Once);
		m_ActionsWidget.Render(0, 0);
		m_Viewport.Render(&m_WindowStates.isViewportOpen, 0, engine);
	}

	if (m_WindowStates.isInspectorOpen) {
		m_Inspector.Render(&m_WindowStates.isInspectorOpen, 0);
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	return Editor::LayoutRenderStatus::SUCCESS;
}

void Editor::MainLayout::OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
	m_Viewport.OnResize(vulkan, engine);
}

void Editor::MainLayout::InitDockspace() {
	if (m_IsDockspaceInitialized) {
		return;
	}

	m_ActionsWidget.InitContexed();
	m_ContentBrowser.InitContexed();
	m_Viewport.InitContexed();

	m_WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	m_WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (m_DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		m_WindowFlags |= ImGuiWindowFlags_NoBackground;

	// Docks building.
	ImGui::DockBuilderRemoveNode(m_DockIDs.MainDock); // clear any previous layout
	ImGui::DockBuilderAddNode(m_DockIDs.MainDock, m_DockspaceFlags | ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(m_DockIDs.MainDock, viewport->Size);

	// Order important!
	m_DockIDs.RightDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Right, 0.2f, nullptr, &m_DockIDs.MainDock);
	m_DockIDs.DownDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Down, 0.2f, nullptr, &m_DockIDs.MainDock);
	m_DockIDs.LeftDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Left, 0.2f, nullptr, &m_DockIDs.MainDock);
	m_DockIDs.TopDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Up, 0.05f, nullptr, &m_DockIDs.MainDock);

	ImGui::DockBuilderDockWindow(Editor::SceneActionsWidget::TITLE, m_DockIDs.TopDock);
	ImGui::DockBuilderDockWindow(Editor::SceneHierarchy::TITLE, m_DockIDs.LeftDock);
	ImGui::DockBuilderDockWindow(Editor::Inspector::TITLE, m_DockIDs.RightDock);
	ImGui::DockBuilderDockWindow(Editor::ContentBrowser::TITLE, m_DockIDs.DownDock);
	ImGui::DockBuilderDockWindow("Viewport", m_DockIDs.MainDock);
	ImGui::DockBuilderFinish(m_DockIDs.MainDock);

	m_IsDockspaceInitialized = true;
}

void Editor::MainLayout::OnContextChanged() {
	m_Context = GameProject::Instance()->GetCurrentContext();

	m_ContentBrowser.SetContext(m_Context);
	m_SceneHierarchy.SetContext(m_Context);
	m_Inspector.SetContext(m_Context);
	m_Viewport.SetContext(m_Context);
	m_LuaConsole.SetContext(m_Context);
	m_ActionsWidget.SetContext(m_Context);
}
