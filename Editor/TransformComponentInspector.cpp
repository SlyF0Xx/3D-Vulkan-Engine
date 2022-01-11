#include "TransformComponentInspector.h"

Editor::TransformComponentInspector::TransformComponentInspector(EDITOR_GAME_TYPE ctx)
	: BaseComponentInspector(ctx) {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SceneDispatcher != nullptr);

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = (entt::entity) e.Entities[0];
		m_TransformComponent = GetComponent<diffusion::TransformComponent>(m_Selection);
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_TransformComponent = nullptr;
	});

	m_SnapDispatcher = ViewportSnapInteractionSingleTon::GetDispatcher();
	IM_ASSERT(&m_SnapDispatcher != nullptr);

	m_SnapDispatcher->appendListener(ViewportInteractType::TRANSFORM_SNAP, [&](const ViewportInteractType& type, const bool enabled, const int value) {
		m_IsTransformSnap = enabled;
		m_TransformSnapSize = static_cast<TransformSnapSize>(value);
	});

	m_SnapDispatcher->appendListener(ViewportInteractType::ROTATION_SNAP, [&](const ViewportInteractType& type, const bool enabled, const int value) {
		m_IsRotationSnap = enabled;
		m_RotationSnapSize = static_cast<RotationSnapSize>(value);
	});

	m_SnapDispatcher->appendListener(ViewportInteractType::SCALE_SNAP, [&](const ViewportInteractType& type, const bool enabled, const int value) {
		m_IsScaleSnap = enabled;
		m_ScaleSnapSize = static_cast<ScaleSnapSize>(value);
	});
}

void Editor::TransformComponentInspector::RenderContent() {
	m_AlreadyAppliedChanges = false;

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(m_TransformComponent->m_world_matrix, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);

	// LOCATION.
	ImGui::BeginGroupPanel("Location", ImVec2(-1.0f, -1.0f));
	if (!m_LocationFocused) {
		m_Location[0] = translation.x;
		m_Location[1] = translation.y;
		m_Location[2] = translation.z;
	}
	ImGui::PushItemWidth(-1);
	if (ImGui::DragFloatN_Colored("##Location", m_Location, 3, GetTransformSpeedBySnapSize(m_IsTransformSnap, m_TransformSnapSize))) {
		ApplyTransform();
	}
	bool focused = ImGui::IsItemActive();
	if (m_LocationFocused && !focused) {
		ApplyTransform();
	}
	m_LocationFocused = focused;
	ImGui::EndGroupPanel();

	// ROTATION.
	ImGui::BeginGroupPanel("Rotation", ImVec2(-1.0f, -1.0f));
	glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));
	if (!m_RotationFocused) {
		m_Rotation[0] = euler.x;
		m_Rotation[1] = euler.y;
		m_Rotation[2] = euler.z;
	}

	ImGui::PushItemWidth(-1);
	if (ImGui::DragFloatN_Colored("##Rotation", m_Rotation, 3, GetRotationSpeedBySnapSize(m_IsRotationSnap, m_RotationSnapSize))) {
		ApplyTransform();
	}
	focused = ImGui::IsItemActive();
	if (m_RotationFocused && !focused) {
		ApplyTransform();
	}
	m_RotationFocused = focused;
	ImGui::EndGroupPanel();

	// SCALE.
	ImGui::BeginGroupPanel("Scale", ImVec2(-1.0f, -1.0f));
	if (!m_ScaleFocused) {
		m_Scale[0] = scale.x;
		m_Scale[1] = scale.y;
		m_Scale[2] = scale.z;
	}
	ImGui::PushItemWidth(-1);
	if (ImGui::DragFloatN_Colored("##Scale", m_Scale, 3, GetScaleSpeedBySnapSize(m_IsScaleSnap, m_ScaleSnapSize), 0.01f)) {
		ApplyTransform();
	}
	focused = ImGui::IsItemActive();
	if (m_ScaleFocused && !focused) {
		ApplyTransform();
	}
	m_ScaleFocused = focused;
	ImGui::EndGroupPanel();
}

void Editor::TransformComponentInspector::OnRegisterUpdated() {
	Editor::BaseComponentInspector::OnRegisterUpdated();
	if (m_Selection != entt::null && m_Context->get_registry().valid(m_Selection)) {
		m_TransformComponent = GetComponent<diffusion::TransformComponent>(m_Selection);
	} else {
		m_TransformComponent = nullptr;
	}
}

inline const char* Editor::TransformComponentInspector::GetTitle() const {
	if (IsRenderable()) {
		return "Transform";
	}
	return "Transform [ERROR]";
}

bool Editor::TransformComponentInspector::IsRenderable() const {
	return m_TransformComponent;
}

void Editor::TransformComponentInspector::ApplyTransform() {
	if (m_AlreadyAppliedChanges) {
		return;
	}

	if (m_Scale[0] == 0.0f) {
		m_Scale[0] += 0.01f;
	}

	if (m_Scale[1] == 0.0f) {
		m_Scale[1] += 0.01f;
	}

	if (m_Scale[2] == 0.0f) {
		m_Scale[2] += 0.01f;
	}

	glm::vec3 loc = glm::make_vec3(m_Location);
	glm::vec3 rotRad = glm::radians(glm::make_vec3(m_Rotation));
	glm::vec3 scale = glm::make_vec3(m_Scale);

#if _DEBUG
	printf("Applied Transform via Inspector\n");
	printf("LOC: %lf %lf %lf\n", loc.x, loc.y, loc.z);
	printf("ROT: %lf %lf %lf\n", rotRad.x, rotRad.y, rotRad.z);
	printf("SCA: %lf %lf %lf\n", scale.x, scale.y, scale.z);
#endif

	INS_COM_REP<diffusion::TransformComponent>(m_Selection, diffusion::create_matrix(loc, rotRad, scale));
	m_AlreadyAppliedChanges = true;
}

void Editor::TransformComponentInspector::OnRemoveComponent() {
	m_Context->get_registry().remove<diffusion::TransformComponent>(m_Selection);
	m_SceneDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});

	Editor::BaseComponentInspector::OnRemoveComponent();
}
