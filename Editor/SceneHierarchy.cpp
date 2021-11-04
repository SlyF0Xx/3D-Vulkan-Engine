#include "SceneHierarchy.h"

Editor::SceneHierarchy::SceneHierarchy(const Ref<Game>& game) {
	SetContext(game);
}

void Editor::SceneHierarchy::SetContext(const Ref<Game>& game) {
	m_Context = game;
}

void Editor::SceneHierarchy::Render() {
	Widget::Render();
}

void Editor::SceneHierarchy::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::Begin(TITLE, p_open, flags);

	m_Context->get_registry().each([&](auto entity) {
		DrawEntityNode((ENTT_ID_TYPE) entity);
	});

	ImGui::End();
}

void Editor::SceneHierarchy::DrawEntityNode(ENTT_ID_TYPE entity) {
	auto tagComponent = this->m_Context->get_registry().try_get<TagComponent>((entt::entity)entity);
	std::string tag;
	if (tagComponent == nullptr) {
		tag = "Object# " + std::to_string(entity);
	} else {
		tag = ((TagComponent*) tagComponent)->m_Tag;
	}

	ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx((void*) (uint64_t) (uint32_t) entity, flags, tag.c_str());
	if (ImGui::IsItemClicked()) {
		m_SelectionContext = entity;
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem()) {
		ImGui::Text("Actions");
		ImGui::Separator();
		if (ImGui::MenuItem("Rename Entity")) {

		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, .2f, .2f, 1.f));
		if (ImGui::MenuItem("Delete")) {
			entityDeleted = true;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	if (opened) {
		ImGui::TreePop();
	}

	if (entityDeleted) {
		// TODO: delete entity from scene.
		if (m_SelectionContext == entity)
			m_SelectionContext = ENTT_ID_TYPE(-1);
	}
}
