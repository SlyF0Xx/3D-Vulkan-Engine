#include "TransformComponentInspector.h"

Editor::TransformComponentInspector::TransformComponentInspector(const diffusion::Ref<Game>& ctx) 
	: BaseComponentInspector(ctx) {
	// ..
}

void Editor::TransformComponentInspector::OnEvent(const SceneInteractEvent& e) {
	BaseComponentInspector::OnEvent(e);

	if (!IsAvailable()) {
		m_TransformComponent = nullptr;
		return;
	}

	m_TransformComponent = GetComponent<diffusion::TransformComponent>(GetFirstEntity());
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
	if (ImGui::DragFloatN_Colored("##Location", m_Location, 3)) {
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
	if (ImGui::DragFloatN_Colored("##Rotation", m_Rotation, 3)) {
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
	if (ImGui::DragFloatN_Colored("##Scale", m_Scale, 3, 0.1f, 0.01f)) {
		ApplyTransform();
	}
	focused = ImGui::IsItemActive();
	if (m_ScaleFocused && !focused) {
		ApplyTransform();
	}
	m_ScaleFocused = focused;
	ImGui::EndGroupPanel();
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
	glm::vec3 rotRad = glm::radians(glm::make_vec3(m_Rotation)) * -1.f;
	glm::vec3 scale = glm::make_vec3(m_Scale);

	/*printf("Applied\n");
	printf("LOC: %lf %lf %lf\n", loc.x, loc.y, loc.z);
	printf("ROT: %lf %lf %lf\n", rot.x, rot.y, rot.z);
	printf("SCA: %lf %lf %lf\n", scale.x, scale.y, scale.z);*/

	INS_COM_REP<diffusion::TransformComponent>(GetFirstEntity(), diffusion::create_matrix(loc, rotRad, scale));
	m_AlreadyAppliedChanges = true;
}
