#include "MainLayout.h"

Editor::MainLayout::MainLayout(diffusion::Ref<Game>& vulkan) {
	m_ContentBrowser = Editor::ContentBrowser();

	m_TextEditor = TextEditor();
	m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	m_TextEditor.SetTabSize(4);
	m_TextEditor.SetPalette(m_TextEditor.GetLightPalette());
	m_TextEditor.SetShowWhitespaces(false);

	m_LuaConsole = LuaConsole();

	m_SceneHierarchy = SceneHierarchy(vulkan);
}

void Editor::MainLayout::Render(Game& vulkan, PresentationEngine& engine) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	bool ds = true;
	ImGui::Begin("DockSpace", &ds, window_flags);

	// DockSpace
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	static auto first_time = true;
	if (first_time) {
		first_time = false;

		ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

		// Order important!
		ImGuiID dock_main_id = dockspace_id;
		ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
		ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
		ImGuiID dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.05f, nullptr, &dock_main_id);
		ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
		ImGuiID dock_down_right_id = ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

		ImGui::DockBuilderDockWindow("Actions", dock_up_id);
		ImGui::DockBuilderDockWindow(Editor::SceneHierarchy::TITLE, dock_left_id);
		ImGui::DockBuilderDockWindow("Right", dock_right_id);
		ImGui::DockBuilderDockWindow(Editor::ContentBrowser::TITLE, dock_down_id);
		ImGui::DockBuilderDockWindow("Project", dock_down_right_id);
		ImGui::DockBuilderDockWindow("Top2", dock_main_id);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGui::End();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("New");
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				//exit(window, vulkan->get_instance(), vulkan->get_device());
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::MenuItem("Content Browser")) {
				m_WindowStates.isContentBrowserOpen = !m_WindowStates.isContentBrowserOpen;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	m_SceneHierarchy.Render();

	if (m_WindowStates.isContentBrowserOpen) {
		m_ContentBrowser.Render(&m_WindowStates.isContentBrowserOpen, 0);
	}

	ImGui::Begin("Top2");
	//ImGui::PushFont(fontCodeEditor);
	m_TextEditor.Render("Editor", ImVec2(0, 0), true);
	//ImGui::PopFont();
	ImGui::End();

	ImGui::Begin("Right");

	ImVec2 current_size = ImGui::GetWindowSize();
	if (current_size.x != m_SceneSize.x || current_size.y != m_SceneSize.y) {
		m_SceneSize = current_size;

		m_TexIDs.clear();
		for (auto& swapchain_data : engine.m_swapchain_data) {
			vk::Sampler color_sampler = vulkan.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
			m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
		}
	}

	ImGui::Image(m_TexIDs[vulkan.get_presentation_engine().FrameIndex], current_size);
	ImGui::Text("Hello, Right!");
	ImGui::End();

	m_LuaConsole.Render();
}
