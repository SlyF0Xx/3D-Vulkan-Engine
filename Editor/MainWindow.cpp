#include "MainWindow.h"
#include "InputEvents.h"
#include "Systems/CameraSystem.h"
#include "BaseComponents/DebugComponent.h"

Editor::MainWindow::MainWindow() : Editor::EditorWindow::EditorWindow() {
	// ..
}

//void Editor::MainWindow::OnContextChanged() {
//	Editor::EditorWindow::OnContextChanged();
//	StartMainLoop();
//}

void Editor::MainWindow::DispatchCameraMovement() {
	
	if (ImGui::GetIO().KeysDown[GLFW_KEY_W]) {
		diffusion::move_forward();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_S]) {
		diffusion::move_backward();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_A]) {
		diffusion::move_left();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_D]) {
		diffusion::move_right();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_SPACE]) {
		diffusion::move_up();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_LEFT_SHIFT]) {
		diffusion::move_down();
	}
	
}
void Editor::MainWindow::DispatchScriptControl() {
	if (ImGui::GetIO().KeysDown[GLFW_KEY_ENTER]) {
		if (m_Context->get_registry().ctx<diffusion::MainCameraTag>().m_entity == m_edittor_camera) {
			m_Context->get_registry().set<diffusion::MainCameraTag>(m_camera);
		}

		m_Context->run();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_BACKSPACE]) {
		if (m_Context->get_registry().ctx<diffusion::MainCameraTag>().m_entity == m_camera) {
			m_Context->get_registry().set<diffusion::MainCameraTag>(m_edittor_camera);
		}

		m_Context->pause();
	}

	if (ImGui::GetIO().KeysDown[GLFW_KEY_ESCAPE]) {
		m_Context->stop();

		m_Context->get_registry().view<diffusion::CameraComponent, diffusion::debug_tag>().each([this](const diffusion::CameraComponent& camera) {
			entt::entity camera_entity = entt::to_entity(m_Context->get_registry(), camera);
			m_Context->get_registry().set<diffusion::MainCameraTag>(camera_entity);
			m_edittor_camera = camera_entity;
		});
	}
}

void Editor::MainWindow::DispatchKeyInputs() {
	DispatchCameraMovement();
	DispatchScriptControl();
}

void Editor::MainWindow::StartMainLoop() {
	if (!m_IsInitialized) return;

	/*auto& main_camera = m_Context->get_registry().ctx<diffusion::MainCameraTag>();
	m_camera = main_camera.m_entity;

	m_edittor_camera = m_Context->get_registry().create();
	m_Context->get_registry().emplace<diffusion::TransformComponent>(m_edittor_camera, diffusion::create_matrix());
	m_Context->get_registry().emplace<diffusion::CameraComponent>(m_edittor_camera);
	m_Context->get_registry().emplace<diffusion::debug_tag>(m_edittor_camera);
	m_Context->get_registry().set<diffusion::MainCameraTag>(m_edittor_camera);*/

	// Systems Initialization
	diffusion::CameraSystem camera_system(m_Context->get_registry());
	diffusion::move_forward.append([&camera_system]() {camera_system.move_forward(0.1f); });
	diffusion::move_backward.append([&camera_system]() {camera_system.move_backward(0.1f); });
	diffusion::move_left.append([&camera_system]() {camera_system.move_left(0.1f); });
	diffusion::move_right.append([&camera_system]() {camera_system.move_right(0.1f); });
	diffusion::move_up.append([&camera_system]() {camera_system.move_up(0.1f); });
	diffusion::move_down.append([&camera_system]() {camera_system.move_down(0.1f); });


	// Main loop
	while (!glfwWindowShouldClose(m_Window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		DispatchKeyInputs();

		// Resize swap chain?
		if (m_SwapChainRebuild) {
			glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
			if (m_Width > 0 && m_Height > 0) {
				ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(
					m_Context->get_instance(),
					m_Context->get_physical_device(),
					m_Context->get_device(),
					&m_MainWindowData,
					m_Context->get_queue_family_index(),
					m_Allocator,
					m_Width,
					m_Height,
					m_MinImageCount
				);
				m_MainWindowData.FrameIndex = 0;

				// TODO: recalculate size
				m_Layout->OnResize(m_Context, *m_PresentationEngine);

				m_SwapChainRebuild = false;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		//if (GameProject::Instance()->IsReady()) {
		if (m_Layout->Render(*m_Context, *m_PresentationEngine) != LayoutRenderStatus::SUCCESS) {
			break;
		}
		//}

		GameProject::Instance()->Render();

		ImGui::Render();
		ImGui::EndFrame();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized) {
			m_MainWindowData.ClearValue.color.float32[0] = m_BackgroundColor.x * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[1] = m_BackgroundColor.y * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[2] = m_BackgroundColor.z * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[3] = m_BackgroundColor.w;

			try {
				m_Context->render_tick();
			} catch (vk::OutOfDateKHRError& out_of_date) {
				m_SwapChainRebuild = true;
			}
		}
	}
}

std::string Editor::MainWindow::GetWindowTitle() const {
	auto gameProject = GameProject::Instance();
	auto scene = gameProject->GetActiveScene();
	if (scene) {
		return gameProject->GetTitle() + " - " + gameProject->GetActiveScene()->GetTitle();
	}
	return "Main Window";
}

