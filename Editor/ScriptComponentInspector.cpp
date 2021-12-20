#include "ScriptComponentInspector.h"

Editor::ScriptComponentInspector::ScriptComponentInspector(EDITOR_GAME_TYPE ctx) : Editor::BaseComponentInspector(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
		m_Component = GetComponent<diffusion::ScriptComponent>(m_Selection);
		if (!m_Component) {
			return;
		}
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_Selection = entt::null;
		m_Component = nullptr;
	});
}

void Editor::ScriptComponentInspector::RenderContent() {
	ImGui::BeginGroupPanel("Script content", ImVec2(-1.0f, -1.0f));
	ImGui::Bullet();
	if (ImGui::Button("Edit")) {
		m_SceneDispatcher->dispatch({SceneInteractType::EDIT_SCRIPT_COMPONENT, (ENTT_ID_TYPE) m_Selection});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove")) {
		m_Context->get_registry().erase<diffusion::ScriptComponent>(m_Selection);
		m_SceneDispatcher->dispatch({SceneInteractType::REMOVE_SCRIPT_COMPONENT, (ENTT_ID_TYPE) m_Selection});
		m_SceneDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});
	}
	ImGui::EndGroupPanel();
}

inline const char* Editor::ScriptComponentInspector::GetTitle() const {
	return "Script Component";
}

bool Editor::ScriptComponentInspector::IsRenderable() const {
	return m_Component;
}
