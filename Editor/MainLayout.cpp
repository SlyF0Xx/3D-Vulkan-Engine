#include "MainLayout.h"
#include <BaseComponents/DebugComponent.h>

Editor::MainLayout::MainLayout(diffusion::Ref<Game>& vulkan) :
	EditorLayout(vulkan),
	m_ContentBrowser(vulkan), 
	m_SceneHierarchy(vulkan), 
	m_Inspector(vulkan),
	m_Viewport(vulkan),
	m_LuaConsole(vulkan)
{

	m_SceneEventDispatcher = CreateRef<SceneEventDispatcherSrc>();

	m_SceneHierarchy.SetDispatcher(m_SceneEventDispatcher);
	m_Inspector.SetDispatcher(m_SceneEventDispatcher);
	m_Inspector.SetSnapDispatcher(m_Viewport.GetDispatcher());

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
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("New");
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
	m_DockIDs.TopDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Up, 0.05f, nullptr, &m_DockIDs.MainDock);
	m_DockIDs.LeftDock = ImGui::DockBuilderSplitNode(m_DockIDs.MainDock, ImGuiDir_Left, 0.2f, nullptr, &m_DockIDs.MainDock);

	ImGui::DockBuilderDockWindow("Actions", m_DockIDs.TopDock);
	ImGui::DockBuilderDockWindow(Editor::SceneHierarchy::TITLE, m_DockIDs.LeftDock);
	ImGui::DockBuilderDockWindow(Editor::Inspector::TITLE, m_DockIDs.RightDock);
	ImGui::DockBuilderDockWindow(Editor::ContentBrowser::TITLE, m_DockIDs.DownDock);
	ImGui::DockBuilderDockWindow("Viewport", m_DockIDs.MainDock);
	ImGui::DockBuilderFinish(m_DockIDs.MainDock);

	m_IsDockspaceInitialized = true;
}

void Editor::MainLayout::ImportScene() {
	std::ifstream fin("sample_scene.json");
	std::string str {std::istreambuf_iterator<char>(fin),
					 std::istreambuf_iterator<char>()};

	NJSONInputArchive json_in(str);
	entt::basic_snapshot_loader loader(m_Context->get_registry());
	loader.entities(json_in)
		.component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
		diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
		diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent, diffusion::PointLightComponent,
		diffusion::debug_tag /* should be ignored in runtime*/>(json_in);

	auto main_entity = m_Context->get_registry().view<diffusion::PossessedEntity>().front();
	m_Context->get_registry().set<diffusion::PossessedEntity>(main_entity);
	m_Context->get_registry().set<diffusion::MainCameraTag>(main_entity);

	// Important.
	m_Viewport.OnSceneUpdated();
}
