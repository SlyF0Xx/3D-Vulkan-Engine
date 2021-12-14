#include "SceneHierarchy.h"

Editor::SceneHierarchy::SceneHierarchy(const Ref<Game>& game) : GameWidget(game) {
	m_SingleDispatcher = SceneInteractionSingleTon::GetDispatcher();
}

void Editor::SceneHierarchy::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::Begin(TITLE, p_open, flags);

	m_Context->get_registry().each([&](auto entity) {
		const auto& relation = m_Context->get_registry().try_get<Relation>(entity);
		if (relation == nullptr) {
			// Only parents or independent objects.
			DrawEntityNode((ENTT_ID_TYPE) entity);
		}
	});

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Constants::EDITOR_WINDOW_PADDING);

	const char* text = "Add Entity";
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	if (ImGui::Button(text, {0.f, 0.f})) {
		ImGui::OpenPopup(POPUP_ADD_ENTITY);
	}

	if (ImGui::BeginPopup(POPUP_ADD_ENTITY)) {
		for (EditorCreatableEntity entity : m_CreatableEntities) {
			DrawCreatableEntityNode(entity);
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			
			import_entity(m_Context->get_registry(), std::filesystem::path(filePathName));
		}

		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::SceneHierarchy::DrawEntityNode(ENTT_ID_TYPE entity) {
	// Renaming entity.
	if (m_IsRenaming && m_SelectionContext == entity) {
		if (!m_IsRenameInputFocused) {
			ImGui::SetKeyboardFocusHere(0);
			m_IsRenameInputFocused = true;
		}

		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("Title", m_RenameBuf, Constants::ACTOR_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (strlen(m_RenameBuf) > 0) {
				m_Context->get_registry()
					.emplace_or_replace<TagComponent, std::string>((entt::entity) m_SelectionContext, std::string(m_RenameBuf));
			}
			StopRenaming();
		}
		ImGui::PopItemWidth();

		if (ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
			StopRenaming();
		}

		return;
	} // Renaming entity.

	auto tagComponent = m_Context->get_registry().try_get<TagComponent>((entt::entity) entity);
	std::string tag;
	if (tagComponent == nullptr) {
		tag = "Object# " + std::to_string(entity);
	} else {
		tag = ((TagComponent*) tagComponent)->m_Tag;
	}

	ImGuiTreeNodeFlags treeNodeFlags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	auto* children = m_Context->get_registry().try_get<Childs>((entt::entity) entity);
	if (children) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isOpened = ImGui::TreeNodeEx((void*) (uint64_t) (uint32_t) entity, treeNodeFlags, tag.c_str());

	if (ImGui::IsItemClicked()) {
		StopRenaming();
		SelectOneNotify(entity);
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

	if (isOpened && children) {
		for (const auto& child : children->m_childs) {
			DrawEntityNode((ENTT_ID_TYPE) child);
		}

		ImGui::TreePop();
	}

	if (isEntityDeleted) {
		// TODO: delete entity from scene.
		if (m_SelectionContext == entity)
			ResetSelectionNotify();
	}


}

void Editor::SceneHierarchy::DrawCreatableEntityNode(EditorCreatableEntity entity) {
	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

	if (entity.Children) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isOpened = ImGui::TreeNodeEx(entity.Title, treeNodeFlags);

	if (ImGui::IsItemClicked() && !entity.Children) {
		switch (entity.Type) {
			case EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_CUBE:
				create_cube_entity_lit(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_PLANE:
				create_plane_entity_lit(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::LIGHT_DIRECTIONAL: {
				create_directional_light_entity(m_Context->get_registry(), glm::vec3(0.0f, 0.0f, 3.0f));
				break;
			}
			case EditorCreatableEntity::EditorCreatableEntityType::DEBUG_CUBE:
				create_debug_cube_entity(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::IMPORTABLE: {
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Model", ".fbx,.obj", ".");
				break;
			}
		}
		ImGui::CloseCurrentPopup();
	}

	if (isOpened && entity.Children) {
		for (auto i = 0; i < entity.Size; i++) {
			DrawCreatableEntityNode(entity.Children[i]);
		}

		ImGui::TreePop();
	}
}

void Editor::SceneHierarchy::StartRenaming(const ENTT_ID_TYPE& entity, const char* tag) {
	SelectOneNotify(entity);
	strcpy_s(m_RenameBuf, Constants::ACTOR_NAME_LENGTH, tag);
	m_IsRenaming = true;
}

void Editor::SceneHierarchy::StopRenaming() {
	m_IsRenaming = false;
	strcpy_s(m_RenameBuf, 1, "");
	m_IsRenameInputFocused = false;
}

void Editor::SceneHierarchy::SelectOneNotify(const ENTT_ID_TYPE& entity) {
	m_SelectionContext = entity;
	IM_ASSERT(&m_SingleDispatcher != nullptr);
	m_SingleDispatcher->dispatch({ SceneInteractType::SELECTED_ONE, entity });
}

void Editor::SceneHierarchy::ResetSelectionNotify() {
	m_SelectionContext = ENTT_ID_TYPE(-1);
	IM_ASSERT(&m_SingleDispatcher != nullptr);
	m_SingleDispatcher->dispatch({ SceneInteractType::RESET_SELECTION });
}
