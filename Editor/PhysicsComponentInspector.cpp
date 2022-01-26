#include "PhysicsComponentInspector.h"

#include "PhysicsUtils.h"

Editor::PhysicsComponentInspector::PhysicsComponentInspector(EDITOR_GAME_TYPE ctx) : Editor::BaseComponentInspector(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
		if (HasMass()) {
			auto& mass = m_Context->get_registry().get<ColliderDefinition>(m_Selection);
			m_MassInKg = mass.mass;
			m_ColliderType = mass.CollisionType;
		}
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_Selection = entt::null;
	});
}

void Editor::PhysicsComponentInspector::RenderContent() {
	if (HasMass()) {
		ImGui::BeginGroupPanel("Mass", ImVec2(-1.0f, -1.0f));
		if (ImGui::DragFloat("kg", &m_MassInKg, 0.1f, 0.01f, 100.f)) {
			m_Context->get_registry().patch<ColliderDefinition>(m_Selection, [&](ColliderDefinition& mass) {
				mass.mass = m_MassInKg;
			});
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel("Collider Type", ImVec2(-1.0f, -1.f));

		static std::map<ECollisionType, std::string> policies;
		policies[ECollisionType::Trigger] = "Trigger";
		policies[ECollisionType::Blocker] = "Blocker";
		policies[ECollisionType::Static] = "Static";
		policies[ECollisionType::Ignore] = "Ignore";

		ImGui::PushItemWidth(-1);
		if (ImGui::BeginCombo("##ColliderType", policies[m_ColliderType].c_str(), 0)) {
			for (auto it = policies.begin(); it != policies.end(); it++) {
				const bool is_selected = (m_ColliderType == it->first);
				if (ImGui::Selectable(it->second.c_str(), is_selected)) {
					m_ColliderType = it->first;
					m_Context->get_registry().patch<ColliderDefinition>(m_Selection, [&](ColliderDefinition& mass) {
						mass.CollisionType = m_ColliderType;
					});
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::EndGroupPanel();
	}
}

void Editor::PhysicsComponentInspector::OnRegisterUpdated() {
	Editor::BaseComponentInspector::OnRegisterUpdated();
	if (m_Selection != entt::null && m_Context->get_registry().valid(m_Selection)) {
		if (HasMass()) {
			auto& mass = m_Context->get_registry().get<ColliderDefinition>(m_Selection);
			m_MassInKg = mass.mass;
		}
	}
}

inline const char* Editor::PhysicsComponentInspector::GetTitle() const {
	return "Physics";
}

void Editor::PhysicsComponentInspector::OnRemoveComponent() {
	if (HasMass()) {
		m_Context->get_registry().remove<ColliderDefinition>(m_Selection);
	}

	m_SceneDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});

	Editor::BaseComponentInspector::OnRemoveComponent();
}

bool Editor::PhysicsComponentInspector::IsRenderable() const {
	return Editor::BaseComponentInspector::IsRenderable() && (HasMass() || false);
}

bool Editor::PhysicsComponentInspector::HasMass() const {
	// TODO: CHECK IF DYNAMIC. (STATIC/TRIGGER/IGNORE - ignore).
	return m_Context->get_registry().view<ColliderDefinition>().contains(m_Selection);
}
