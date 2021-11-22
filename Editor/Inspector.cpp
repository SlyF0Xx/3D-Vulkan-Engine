#include "Inspector.h"

Editor::Inspector::Inspector(const Ref<Game>& game) 
	: GameWidget(game), m_TagInspector(game), m_TransformInspector(game) {
	// ..
}

void Editor::Inspector::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	ImGui::Begin("Inspector", p_open, flags);

	if (!m_IsSelected) {
		ImGui::Text("Nothing to inspect.");
		ImGui::End();
		ImGui::PopStyleVar();
		return;
	}

	
	m_TagInspector.Render();
	m_TransformInspector.Render();

	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::Inspector::SetDispatcher(const SceneEventDispatcher& dispatcher) {
	m_SingleDispatcher = SceneEventDispatcher(dispatcher);

	IM_ASSERT(&m_SingleDispatcher != nullptr);

	m_SingleDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& sEvent) {
		m_IsSelected = true;
		OnEvent(sEvent);
	});

	m_SingleDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& sEvent) {
		m_IsSelected = false;
		OnEvent(sEvent);
	});
}

void Editor::Inspector::SetSnapDispatcher(const ViewportEventDispatcher& dispatcher) {
	m_TransformInspector.SetSnapDispatcher(dispatcher);
}

void Editor::Inspector::OnEvent(const SceneInteractEvent& sEvent) {
	m_TagInspector.OnEvent(sEvent);
	m_TransformInspector.OnEvent(sEvent);
}
