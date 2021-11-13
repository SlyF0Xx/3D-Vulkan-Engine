#include "BaseComponentInspector.h"

Editor::BaseComponentInspector::BaseComponentInspector(const diffusion::Ref<Game>& ctx) {
	SetContext(ctx);
}

void Editor::BaseComponentInspector::SetContext(const diffusion::Ref<Game>& game) {
	m_Context = game;
}

void Editor::BaseComponentInspector::OnEvent(const SceneInteractEvent& e) {
	m_Selection = e;
}

void Editor::BaseComponentInspector::Render() {
	if (IsRenderable()) {
		if (ImGui::CollapsingHeader(GetTitle(), ImGuiTreeNodeFlags_DefaultOpen)) {
			/*ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild(GetTitle(), ImVec2(0, 0), ImGuiWindowFlags_AlwaysAutoResize);*/

			RenderContent();

			/*ImGui::EndChild();
			ImGui::PopStyleVar();*/
		}
	}
}

bool Editor::BaseComponentInspector::IsAvailable() const {
	return &m_Selection && ((m_Selection.Length > 1 && MULTI_ENTITIES_SUPPORT) || (m_Selection.Length == 1 && !MULTI_ENTITIES_SUPPORT));
}

bool Editor::BaseComponentInspector::IsRenderable() const {
	return true;
}

entt::entity Editor::BaseComponentInspector::GetFirstEntity() const {
	return (entt::entity) m_Selection.Entities[0];
}
