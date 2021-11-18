#include "MainLayout.h"

Editor::MainLayout::MainLayout(diffusion::Ref<Game>& vulkan) :
	m_ContentBrowser(vulkan), 
	m_SceneHierarchy(vulkan), 
	m_Inspector(vulkan) {

	m_SceneEventDispatcher = CreateRef<SceneEventDispatcherSrc>();

	m_SceneHierarchy.SetDispatcher(m_SceneEventDispatcher);
	m_Inspector.SetDispatcher(m_SceneEventDispatcher);

	m_TextEditor = TextEditor();
	m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	m_TextEditor.SetTabSize(4);
	m_TextEditor.SetPalette(m_TextEditor.GetLightPalette());
	m_TextEditor.SetShowWhitespaces(false);

	m_LuaConsole = LuaConsole();
}

Editor::LayoutRenderStatus Editor::MainLayout::Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
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
		ImGui::Begin("Viewport", &m_WindowStates.isViewportOpen, 0);
		ImVec2 current_size = ImGui::GetWindowSize();
		if (current_size.x != m_SceneSize.x || current_size.y != m_SceneSize.y) {
			m_SceneSize = current_size;

			OnResize(vulkan, engine);
		}

		ImGui::Image(m_TexIDs[vulkan.get_presentation_engine().SemaphoreIndex], current_size);
		ImGui::End();
	}

	if (m_WindowStates.isInspectorOpen) {
		m_Inspector.Render(&m_WindowStates.isInspectorOpen, 0);
	}

	return Editor::LayoutRenderStatus::SUCCESS;
}

void Editor::MainLayout::OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
	engine.resize(m_SceneSize.x, m_SceneSize.y);
	vulkan.InitializePresentationEngine(engine.get_presentation_engine());
	vulkan.SecondInitialize();

	m_TexIDs.clear();
	for (auto& swapchain_data : engine.get_presentation_engine().m_swapchain_data) {
		vk::Sampler color_sampler = vulkan.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}
}

ImVec2 Editor::MainLayout::GetSceneSize() const {
	return m_SceneSize;
}

void Editor::MainLayout::InitDockspace() {
	if (m_IsDockspaceInitialized) {
		return;
	}

	m_ContentBrowser.InitContexed();

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
