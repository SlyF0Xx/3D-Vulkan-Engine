#include "MainWindow.h"

void Editor::MainWindow::StartMainLoop() {
	if (!m_IsInitialized) return;
	// Main loop
	while (!glfwWindowShouldClose(m_Window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

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
				m_Layout->OnResize(*m_Context, *m_PresentationEngine);

				m_SwapChainRebuild = false;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (m_Layout->Render(*m_Context, *m_PresentationEngine) != RENDER_STATUS::SUCCESS) {
			break;
		}

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized) {
			m_MainWindowData.ClearValue.color.float32[0] = m_BackgroundColor.x * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[1] = m_BackgroundColor.y * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[2] = m_BackgroundColor.z * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[3] = m_BackgroundColor.w;

			try {
				m_Context->DrawRestruct();
			} catch (vk::OutOfDateKHRError& out_of_date) {
				m_SwapChainRebuild = true;
			}
		}
	}
}

const char* Editor::MainWindow::GetWindowTitle() const {
	return "Main Window";
}

