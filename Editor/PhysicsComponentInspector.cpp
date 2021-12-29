#include "PhysicsComponentInspector.h"

Editor::PhysicsComponentInspector::PhysicsComponentInspector(EDITOR_GAME_TYPE ctx) : Editor::BaseComponentInspector(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_Selection = entt::null;
	});
}

void Editor::PhysicsComponentInspector::RenderContent() {
	// ..
}

inline const char* Editor::PhysicsComponentInspector::GetTitle() const {
	return "Physics";
}

void Editor::PhysicsComponentInspector::OnRemoveComponent() {
	throw;
}

bool Editor::PhysicsComponentInspector::IsRenderable() const {
	return Editor::BaseComponentInspector::IsRenderable();
}
