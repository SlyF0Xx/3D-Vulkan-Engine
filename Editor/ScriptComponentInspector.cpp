#include "ScriptComponentInspector.h"

Editor::ScriptComponentInspector::ScriptComponentInspector(EDITOR_GAME_TYPE ctx) 
	: Editor::BaseComponentInspector(ctx), m_BTEditor(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
		m_BTEditor.SetSelection(m_Selection);
		m_Component = GetComponent<diffusion::ScriptComponent>(m_Selection);
		if (!m_Component) {
			return;
		}
		m_SizeStr = GetSize();
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_Selection = entt::null;
		m_Component = nullptr;
		m_SizeStr = "";
		m_BTEditor.SetSelection(entt::null);
	});

	m_SceneDispatcher->appendListener(SceneInteractType::UPDATE_SCRIPT_INFO, [&](const SceneInteractEvent& e) {
		m_SizeStr = GetSize();
	});
}

void Editor::ScriptComponentInspector::SetContext(EDITOR_GAME_TYPE game) {
	Editor::BaseComponentInspector::SetContext(game);
	m_BTEditor.SetContext(game);
}

void Editor::ScriptComponentInspector::OnRegisterUpdated() {
	Editor::BaseComponentInspector::OnRegisterUpdated();
	if (m_Selection != entt::null && m_Context->get_registry().valid(m_Selection)) {
		m_Component = GetComponent<diffusion::ScriptComponent>(m_Selection);
		m_BTEditor.OnRegisterUpdated();
		m_BTEditor.SetSelection(m_Selection);
	} else {
		m_Component = nullptr;
	}
}

void Editor::ScriptComponentInspector::RenderContent() {
	ImGui::BeginGroupPanel("Script Content", ImVec2(-1.0f, -1.0f));
	ImGui::Bullet();
	if (ImGui::Button("Modify")) {
		m_SceneDispatcher->dispatch({SceneInteractType::EDIT_SCRIPT_COMPONENT, (ENTT_ID_TYPE) m_Selection});
	}
	ImGui::SameLine();
	ImGui::Text(m_SizeStr.c_str());
	ImGui::EndGroupPanel();

	m_BTEditor.Render(0, 0);
}

inline const char* Editor::ScriptComponentInspector::GetTitle() const {
	return "Script Component";
}

void Editor::ScriptComponentInspector::OnRemoveComponent() {
	m_Context->get_registry().remove<BTComponent>(m_Selection);
	m_Context->get_registry().remove<BTCooldownComponent>(m_Selection);

	m_Context->get_registry().remove<diffusion::ScriptComponent>(m_Selection);
	m_Context->get_registry().remove<diffusion::ScriptComponentState>(m_Selection);
	m_SceneDispatcher->dispatch({SceneInteractType::REMOVE_SCRIPT_COMPONENT, (ENTT_ID_TYPE) m_Selection});
	m_SceneDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});

	Editor::BaseComponentInspector::OnRemoveComponent();
}

bool Editor::ScriptComponentInspector::IsRenderable() const {
	return Editor::BaseComponentInspector::IsRenderable() && m_Component;
}

std::string Editor::ScriptComponentInspector::GetSize() const {
	if (!IsRenderable()) {
		return "";
	}
	std::string representation = " bytes";
	size_t size = m_Component->m_content.length();

	if (m_Component->m_content.length() >= 1024) {
		representation = " KB";
		size = size >> 10;
		if (m_Component->m_content.length() >= (1024 * 1024)) {
			representation = " MB";
			size = size >> 10;
		}
	}

	return "Size: " + std::to_string(size) + representation;
}
