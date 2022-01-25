#include "MainLayout.h"

Editor::MainLayout::MainLayout() :
	EditorLayout(),
	m_ContentBrowser(m_Context),
	m_SceneHierarchy(m_Context),
	m_Inspector(m_Context),
	m_Viewport(m_Context),
	m_LuaConsole(m_Context),
	m_ActionsWidget(m_Context) {
	m_SceneDispatcher = Editor::SceneInteractionSingleTon::GetDispatcher();

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::EDIT_SCRIPT_COMPONENT, [&](const Editor::SceneInteractEvent& event) {
		entt::entity entity = (entt::entity) event.Entities[0];
		if (!m_CodeEditors.contains(entity)) {
			m_CodeEditors[entity] = new CodeEditor(entity, m_Context);
		} else {
			MakeTabVisible(m_CodeEditors[entity]->GetTitle().c_str());
		}
		m_WindowStates.ScriptEditorStates[entity] = true;
	});

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::REMOVE_SCRIPT_COMPONENT, [&](const Editor::SceneInteractEvent& event) {
		entt::entity entity = (entt::entity) event.Entities[0];
		auto position = m_CodeEditors.find(entity);
		if (position != m_CodeEditors.end()) {
			m_CodeEditors.erase(position);
		}
		m_WindowStates.ScriptEditorStates.erase(entity);
	});

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::RUN, [&](const Editor::SceneInteractEvent& event) {
		if (!m_IsScriptEditing)
			MakeTabVisible(EditorViewport::TITLE);
	});

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::STOP, [&](const Editor::SceneInteractEvent& event) {
		while (!m_Context->m_initialized) {}
		GameProject::Instance()->GetActiveScene()->RefreshImGuiBindings();
		OnRegisterUpdated();
		//m_SceneDispatcher->dispatch(Editor::SceneInteractType::RESET_SELECTION);
	});

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::SAVE_SCRIPT, [&](const Editor::SceneInteractEvent& event) {
		m_CodeEditors[m_ScriptEditingEntity]->Save();
		m_SceneDispatcher->dispatch(Editor::SceneInteractType::UPDATE_SCRIPT_INFO);
	});

	m_SceneDispatcher->appendListener(Editor::SceneInteractType::SAVE_ALL_SCTIPTS, [&](const Editor::SceneInteractEvent& event) {
		std::for_each(m_CodeEditors.begin(), m_CodeEditors.end(), [&](std::pair<const entt::entity, CodeEditor*>& pair) {
			pair.second->Save();
		});
		m_SceneDispatcher->dispatch(Editor::SceneInteractType::UPDATE_SCRIPT_INFO);
	});
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

	InitDockspace();

	if (ImGui::BeginMainMenuBar()) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		if (ImGui::BeginMenu("File")) {
			EDITOR_BEGIN_DISABLE_IF_RUNNING
				if (ImGui::MenuItem("New Scene", "CTRL + N")) {
					// Destroy current, because created new one.
					// GetParent()->Destroy();

					ImGui::EndMenu();
					ImGui::EndMainMenuBar();

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();

					return Editor::LayoutRenderStatus::NEW_SCENE;
				}
			ImGui::Separator();
			if (ImGui::MenuItem("Load project", "CTRL + O")) {
				Editor::GameProject::Instance()->Load();
			}
			if (ImGui::MenuItem("Load old project")) {
				Editor::GameProject::Instance()->Load();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Rename Scene", "CTRL + R")) {
				Editor::GameProject::Instance()->RenameScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Project", "CTRL + S")) {
				Editor::GameProject::Instance()->Save();
			}
			if (ImGui::MenuItem("Save Project as...", "CTRL + SHIFT + S")) {
				Editor::GameProject::Instance()->SaveAs();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "ALT + F4")) {
				// GetParent()->Destroy();

				ImGui::EndMenu();
				ImGui::EndMainMenuBar();

				ImGui::PopStyleVar();
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				return Editor::LayoutRenderStatus::EXIT;
			}
			EDITOR_END_DISABLE_IF_RUNNING
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
			EDITOR_BEGIN_DISABLE_IF_RUNNING;
			for (const Scene& scene : GameProject::Instance()->GetScenes()) {
				if (ImGui::MenuItem(scene.GetTitle().c_str(), NULL,
					scene.GetID() == GameProject::Instance()->GetActiveScene()->GetID())) {
					// GameProject::Instance()->SetActiveScene(scene.GetID());

					// Destroy current, because created new one.
					// GetParent()->Destroy();

					if (GameProject::Instance()->PrepareChangeScene(scene.GetID())) {

						ImGui::EndMenu();
						ImGui::EndMainMenuBar();

						ImGui::PopStyleVar();
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();

						return Editor::LayoutRenderStatus::SWITCH_SCENE;
					}
				}
			}

			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, Constants::ERROR_COLOR);
			if (ImGui::MenuItem("[X] Delete Current Scene")) {
				GameProject::Instance()->DeleteScene();
			}
			ImGui::PopStyleColor();
			EDITOR_END_DISABLE_IF_RUNNING;
			ImGui::EndMenu();
		}

		if (m_IsScriptEditing && ImGui::BeginMenu("Scripting")) {
			EDITOR_BEGIN_DISABLE_IF_RUNNING;
			if (ImGui::MenuItem("Save script", "CTRL + ALT + S")) {
				m_SceneDispatcher->dispatch(SceneInteractType::SAVE_SCRIPT);
			}

			if (ImGui::MenuItem("Save all scripts", "CTRL + SHIFT + ALT + S")) {
				m_SceneDispatcher->dispatch(SceneInteractType::SAVE_ALL_SCTIPTS);
			}
			EDITOR_END_DISABLE_IF_RUNNING;
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

	m_IsScriptEditing = false;
	m_ScriptEditingEntity = entt::null;
	ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY));
	std::for_each(m_CodeEditors.begin(), m_CodeEditors.end(), [&](std::pair<const entt::entity, CodeEditor*>& pair) {
		if (m_WindowStates.ScriptEditorStates[pair.first]) {
			ImGui::SetNextWindowDockID(m_DockIDs.MainDock, ImGuiCond_Once);
			pair.second->Render(&m_WindowStates.ScriptEditorStates[pair.first], 0);

			if (IsTabSelected(pair.second->GetID())) {
				m_IsScriptEditing = true;
				m_ScriptEditingEntity = pair.first;
			}

			if (!m_WindowStates.ScriptEditorStates[pair.first]) {
				pair.second->Save();
			}
		}
	});
	ImGui::PopFont();

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

	GameProjectRenderStatus status = GameProject::Instance()->Render();
	if (status != Editor::GameProjectRenderStatus::SUCCESS) {
		if (status == Editor::GameProjectRenderStatus::LOAD_PROJECT) {
			return Editor::LayoutRenderStatus::LOAD_PROJECT;
		} else if (status == Editor::GameProjectRenderStatus::DELETE_SCENE) {
			return Editor::LayoutRenderStatus::DELETE_SCENE;
		}
	}

	return Editor::LayoutRenderStatus::SUCCESS;
}

void Editor::MainLayout::OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) {
	m_Viewport.OnResize(vulkan, engine);
}

void Editor::MainLayout::InitDockspace() {
	if (!m_IsDockspaceInitialized) {
		m_WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		m_WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (m_DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			m_WindowFlags |= ImGuiWindowFlags_NoBackground;
	}

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::Begin("DockSpace", &m_WindowStates.isDocksSpaceOpen, m_WindowFlags);
	m_DockIDs.MainDock = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockIDs.MainDock, ImVec2(0.0f, 0.0f), m_DockspaceFlags);

	if (m_IsDockspaceInitialized) {
		ImGui::End();
		return;
	}

	m_ActionsWidget.InitContexed();
	m_ContentBrowser.InitContexed();
	m_Viewport.InitContexed();

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

	ImGui::End();
}

void Editor::MainLayout::OnContextChanged() {
	m_IsDockspaceInitialized = false;
	m_Context = GameProject::Instance()->GetCurrentContext();

	m_ContentBrowser.SetContext(m_Context);
	m_SceneHierarchy.SetContext(m_Context);
	m_Inspector.SetContext(m_Context);
	m_Viewport.SetContext(m_Context);
	m_LuaConsole.SetContext(m_Context);
	m_ActionsWidget.SetContext(m_Context);
	m_CodeEditors.clear();
	m_ScriptEditingEntity = entt::null;
}

void Editor::MainLayout::OnRegisterUpdated() {
	m_ContentBrowser.OnRegisterUpdated();
	m_SceneHierarchy.OnRegisterUpdated();
	m_Inspector.OnRegisterUpdated();
	m_Viewport.OnRegisterUpdated();
	m_LuaConsole.OnRegisterUpdated();
	m_ActionsWidget.OnRegisterUpdated();
}

diffusion::Ref<Editor::EditorLayout> Editor::MainLayout::Copy() {
	return diffusion::CreateRef<Editor::MainLayout>();
}

void Editor::MainLayout::MakeTabVisible(const char* window_name) {
	ImGuiWindow* window = ImGui::FindWindowByName(window_name);
	if (window == NULL || window->DockNode == NULL || window->DockNode->TabBar == NULL)
		return;
	window->DockNode->TabBar->NextSelectedTabId = window->ID;
}

bool Editor::MainLayout::IsTabSelected(ImGuiID id) {
	ImGuiWindow* window = ImGui::FindWindowByID(id);
	if (window == NULL || window->DockNode == NULL || window->DockNode->TabBar == NULL)
		return false;
	// printf((std::to_string(window->DockNode->TabBar->SelectedTabId) + "\t" + std::to_string(id) + "\n").c_str());
	return window->DockNode->TabBar->SelectedTabId == window->ID;
}
