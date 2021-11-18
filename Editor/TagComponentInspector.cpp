#include "TagComponentInspector.h"

Editor::TagComponentInspector::TagComponentInspector(const diffusion::Ref<Game>& ctx) 
	: BaseComponentInspector(ctx) {
	// ..
}

void Editor::TagComponentInspector::OnEvent(const SceneInteractEvent& e) {
	BaseComponentInspector::OnEvent(e);

	if (!IsAvailable()) {
		m_TagComponent = nullptr;
		return;
	}

	m_TagComponent = GetComponent<diffusion::TagComponent>(GetFirstEntity());
	if (!m_TagComponent) {
		return;
	}

	strcpy_s(m_RenameBuf, Constants::ACTOR_NAME_LENGTH, m_TagComponent->m_Tag.c_str());
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

void Editor::TagComponentInspector::Rename() {
	if (strlen(m_RenameBuf) > 0) {
		INS_COM_REP<diffusion::TagComponent, std::string>(GetFirstEntity(), std::string(m_RenameBuf));
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
