#include "Inspector.h"

Editor::Inspector::Inspector(const Ref<Game>& game) 
	: GameWidget(game), m_TagInspector(game), m_TransformInspector(game) {
	m_SingleDispatcher = SceneInteractionSingleTon::GetDispatcher();

	IM_ASSERT(&m_SingleDispatcher != nullptr);

	m_SingleDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& sEvent) {
		m_IsSelected = true;
	});

	m_SingleDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& sEvent) {
		m_IsSelected = false;
	});
}

void Editor::Inspector::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	ImGui::Begin("Inspector", p_open, flags);

	if (!m_IsSelected) {
		ImGui::Text("Nothing to inspect.");
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		return;
	}

	m_TagInspector.Render();
	m_TransformInspector.Render();

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}
