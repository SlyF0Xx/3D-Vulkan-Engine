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
	auto tagComponent = this->m_Context->get_registry().try_get<TagComponent>((entt::entity) entity);
	std::string tag;
	if (tagComponent == nullptr) {
		tag = "Object# " + std::to_string(entity);
	} else {
		tag = ((TagComponent*) tagComponent)->m_Tag;
	}

	// Renaming entity.
	if (m_IsRenaming && m_SelectionContext == entity) {
		if (!m_IsRenameInputFocused) {
			ImGui::SetKeyboardFocusHere(0);
			m_IsRenameInputFocused = true;
		}

		if (ImGui::InputText("Title", m_RenameBuf, RENAME_BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (strlen(m_RenameBuf) > 0) {
				m_Context->get_registry()
					.emplace_or_replace<TagComponent, std::string>((entt::entity) m_SelectionContext, std::string(m_RenameBuf));
			}
			StopRenaming();
		}

		if (ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
			StopRenaming();
		}

		return;
	} // Renaming entity.

	ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool isOpened = ImGui::TreeNodeEx((void*) (uint64_t) (uint32_t) entity, flags, tag.c_str());

	if (ImGui::IsItemClicked()) {
		StopRenaming();
		m_SelectionContext = entity;
	}

	bool isEntityDeleted = false;
	if (ImGui::BeginPopupContextItem()) {
		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Actions");
		ImGui::PopFont();
		ImGui::Separator();

		std::string itemName = "Rename " + tag;
		if (ImGui::MenuItem(itemName.c_str())) {
			StartRenaming(entity, tag.c_str());
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, .2f, .2f, 1.f));
		if (ImGui::MenuItem("Delete")) {
			isEntityDeleted = true;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	if (isOpened) {
		ImGui::TreePop();
	}

	if (isEntityDeleted) {
		// TODO: delete entity from scene.
		if (m_SelectionContext == entity)
			m_SelectionContext = ENTT_ID_TYPE(-1);
	}


}

void Editor::SceneHierarchy::StartRenaming(ENTT_ID_TYPE& entity, const char* tag) {
	m_SelectionContext = entity;
	strcpy_s(m_RenameBuf, RENAME_BUF_SIZE, tag);
	m_IsRenaming = true;
}

void Editor::SceneHierarchy::StopRenaming() {
	m_IsRenaming = false;
	strcpy_s(m_RenameBuf, 1, "");
	m_IsRenameInputFocused = false;
}
