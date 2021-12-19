#include "SceneActionsWidget.h"

Editor::SceneActionsWidget::SceneActionsWidget(EDITOR_GAME_TYPE ctx) : Editor::GameWidget(ctx) {
	m_IsStopped = m_Context->m_stopped;
	m_IsPaused = m_Context->m_paused;
}

void Editor::SceneActionsWidget::Render(bool* p_open, ImGuiWindowFlags flags) {

	ImGuiWindowClass window_class;
	window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverMe;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverOther;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitMe;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitOther;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeY;
	window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoSplit;
	ImGui::SetNextWindowClass(&window_class);

	ImGui::Begin(TITLE, p_open, 
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_AlwaysUseWindowPadding
		| flags
	);

	int frame_padding = 4;	 // -1 == uses default padding (style.FramePadding)
	float s = ImGui::GetWindowHeight() - 4.f;
	ImVec2 size = ImVec2(32, 32);     // Size of the image we want to make visible
	ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
	ImVec2 uv1 = ImVec2(1, 1);								// UV coordinates for (thumbnailSize, thumbnailSize) in our texture
	ImVec4 bg_col = ImVec4(0.f, 0.f, 0.f, .0f);         // Background.
	ImVec4 tint_col = ImVec4(1.0f, 1.f, 1.0f, 1.f);       // No tint

	ImTextureID icon = m_IsStopped ? m_PlayTex : m_StopTex;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size.x));
	if (ImGui::ImageButton(icon, size, uv0, uv1, frame_padding, bg_col, tint_col)) {
		if (m_IsStopped)
			m_Context->run();
		else
			m_Context->stop();
		m_IsStopped = !m_IsStopped;
		m_IsPaused = false;
	}

	// Pause button.
	if (m_IsPaused) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_SUCCESS_COLOR);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_DEFAULT_COLOR);
	}

	ImGui::SameLine();

	if (ImGui::ImageButton(m_PauseTex, size, uv0, uv1, frame_padding, bg_col, tint_col)) {
		if (m_IsPaused)
			m_Context->resume();
		else
			m_Context->pause();
		m_IsPaused = !m_IsPaused;
	}

	ImGui::PopStyleColor();
	ImGui::End();
}

void Editor::SceneActionsWidget::InitContexed() {
	Editor::GameWidget::InitContexed();

	m_PlayTex = GenerateTextureID(m_Context, m_TexPlayData, PLAY_ICON_PATH);
	m_StopTex = GenerateTextureID(m_Context, m_StopTexData, STOP_ICON_PATH);
	m_PauseTex = GenerateTextureID(m_Context, m_TexPauseData, PAUSE_ICON_PATH);
}
