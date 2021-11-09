#include "Inspector.h"

Editor::Inspector::Inspector(const Ref<Game>& game) : GameWidget(game) {
	// ..
}

void Editor::Inspector::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::Begin("Inspector", p_open, flags);

	if (!m_IsSelected) {
		ImGui::Text("Nothing to inspect.");
		ImGui::End();
		return;
	}

	auto tagComponent = m_Context->get_registry().try_get<TagComponent>((entt::entity) m_SelectionContext);
	if (tagComponent) {
		ImGui::Text("Has TagComponent");
	}

	auto cameraComponent = m_Context->get_registry().try_get<CameraComponent>((entt::entity) m_SelectionContext);
	if (cameraComponent) {
		ImGui::Text("Has CameraComponent");
	}

	auto transformComponent = m_Context->get_registry().try_get<TransformComponent>((entt::entity) m_SelectionContext);
	if (transformComponent) {
		ImGui::Text("Has TransformComponent");
	}

	ImGui::End();
}

void Editor::Inspector::SetDispatcher(const SceneEventDispatcher& dispatcher) {
	m_SingleDispatcher = SceneEventDispatcher(dispatcher);

	IM_ASSERT(&m_SingleDispatcher != nullptr);

	m_SingleDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& sEvent) {
		m_SelectionContext = sEvent.Entities[0];
		m_IsSelected = true;
	});

	m_SingleDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& sEvent) {
		m_SelectionContext = ENTT_ID_TYPE(-1);
		m_IsSelected = false;
	});
}
