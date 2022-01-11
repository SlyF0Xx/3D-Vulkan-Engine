#include "TagComponentInspector.h"

Editor::TagComponentInspector::TagComponentInspector(EDITOR_GAME_TYPE ctx)
	: BaseComponentInspector(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
		m_TagComponent = GetComponent<diffusion::TagComponent>(m_Selection);
		if (!m_TagComponent) {
			return;
		}

		strcpy_s(m_RenameBuf, Constants::ACTOR_NAME_LENGTH, m_TagComponent->m_Tag.c_str());
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_TagComponent = nullptr;
	});
}

void Editor::TagComponentInspector::RenderContent() {
	ImGui::PushItemWidth(-1);
	if (ImGui::InputText("Title", m_RenameBuf, Constants::ACTOR_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue)) {
		Rename();
	}

	bool focused = ImGui::IsItemActive();
	if (m_IsFocused && !focused) {
		Rename();
	}
	m_IsFocused = focused;
}

void Editor::TagComponentInspector::OnRegisterUpdated() {
	Editor::BaseComponentInspector::OnRegisterUpdated();
	m_TagComponent = GetComponent<diffusion::TagComponent>(m_Selection);
}

void Editor::TagComponentInspector::Rename() {
	if (strlen(m_RenameBuf) > 0) {
		INS_COM_REP<diffusion::TagComponent, std::string>(m_Selection, std::string(m_RenameBuf));
	} else {
		strcpy_s(m_RenameBuf, Constants::ACTOR_NAME_LENGTH, m_TagComponent->m_Tag.c_str());
	}
}

inline const char* Editor::TagComponentInspector::GetTitle() const {
	if (IsRenderable()) {
		return "Tag";
	}
	return "Tag [ERROR]";
}

bool Editor::TagComponentInspector::IsRenderable() const {
	return m_TagComponent;
}

void Editor::TagComponentInspector::OnRemoveComponent() {
	m_Context->get_registry().remove<diffusion::TagComponent>(m_Selection);
	m_SceneDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});

	Editor::BaseComponentInspector::OnRemoveComponent();
}
