#include "BaseComponentInspector.h"

Editor::BaseComponentInspector::BaseComponentInspector(EDITOR_GAME_TYPE ctx) {
	SetContext(ctx);
}

void Editor::BaseComponentInspector::SetContext(EDITOR_GAME_TYPE game) {
	m_Context = game;
	m_Selection = entt::entity();
}

void Editor::BaseComponentInspector::Render() {
	if (IsRenderable()) {
		if (ImGui::CollapsingHeader(GetTitle(), ImGuiTreeNodeFlags_DefaultOpen)) {
			/*ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild(GetTitle(), ImVec2(0, 0), ImGuiWindowFlags_AlwaysAutoResize);*/
			EDITOR_BEGIN_DISABLE_IF_RUNNING
			RenderContent();
			EDITOR_END_DISABLE_IF_RUNNING

			/*ImGui::EndChild();
			ImGui::PopStyleVar();*/
		}
	}
}

bool Editor::BaseComponentInspector::IsAvailable() const {
	//return &m_Selection && ((m_Selection.Length > 1 && MULTI_ENTITIES_SUPPORT) || (m_Selection.Length == 1 && !MULTI_ENTITIES_SUPPORT));
	return true;
}

bool Editor::BaseComponentInspector::IsRenderable() const {
	return true;
}
