#include "CodeEditor.h"

Editor::CodeEditor::CodeEditor(entt::entity entity, EDITOR_GAME_TYPE ctx) : Editor::GameWidget(ctx) {
	m_TextEditor = TextEditor();
	m_TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	m_TextEditor.SetTabSize(4);
	m_TextEditor.SetPalette(m_TextEditor.GetLightPalette());
	m_TextEditor.SetShowWhitespaces(false);

	SetSelection(entity);
}

void Editor::CodeEditor::SetContext(EDITOR_GAME_TYPE ctx) {
	Editor::GameWidget::SetContext(ctx);
	m_Selection = entt::null;
}

void Editor::CodeEditor::SetSelection(entt::entity entity) {
	m_Selection = entity;

	auto scriptComponent = m_Context->get_registry().try_get<diffusion::ScriptComponent>(m_Selection);
	m_TextEditor.SetText(scriptComponent->m_content);
}

void Editor::CodeEditor::Save() {
	printf(m_TextEditor.GetText().c_str());
	m_Context->get_registry().emplace_or_replace<diffusion::ScriptComponent>(m_Selection, m_TextEditor.GetText());
}

std::string Editor::CodeEditor::GetTitle() const {
	if (m_Selection == entt::null) {
		return "Code Editor [ERROR]";
	}
	auto tagComponent = m_Context->get_registry().try_get<diffusion::TagComponent>(m_Selection);
	std::string tag;
	if (tagComponent == nullptr) {
		tag = "Object #" + std::to_string((ENTT_ID_TYPE) m_Selection);
	} else {
		tag = ((diffusion::TagComponent*) tagComponent)->m_Tag;
	}
	return "Code editor - " + tag;
}

ImGuiID Editor::CodeEditor::GetID() const {
	return ImHashStr(GetTitle().c_str());
}

void Editor::CodeEditor::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::Begin(GetTitle().c_str(), p_open, flags);
	m_TextEditor.Render("Editor", ImVec2(0, 0), true);
	ImGui::End();
}
