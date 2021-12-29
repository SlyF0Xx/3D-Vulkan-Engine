#include "SceneHierarchy.h"

Editor::SceneHierarchy::SceneHierarchy(EDITOR_GAME_TYPE game) : GameWidget(game) {
	m_SingleDispatcher = SceneInteractionSingleTon::GetDispatcher();
}

void Editor::SceneHierarchy::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::Begin(TITLE, p_open, flags);

#if _DEBUG && 0
	ImGui::Text("Only Debug:");
	ImGui::Text("Addr: %p", m_Context);
	ImGui::Text("Entities count: %d", m_Context->get_registry().alive());
#endif

	m_Context->get_registry().each([&](auto entity) {
		const auto& relation = m_Context->get_registry().try_get<Relation>(entity);
		if (!relation) {
			// Only parents or independent objects.
			DrawEntityNode((ENTT_ID_TYPE) entity);
		}
	});

	if (m_Context->get_registry().valid(m_UngroupEntity)) {
		unbind_entity(m_Context->get_registry(), m_UngroupEntity);
		m_UngroupEntity = entt::null;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Constants::EDITOR_WINDOW_PADDING);

	const char* text = "Add Entity";
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	EDITOR_BEGIN_DISABLE_IF_RUNNING;
	if (ImGui::Button(text, {0.f, 0.f})) {
		ImGui::OpenPopup(POPUP_ADD_ENTITY);
	}
	EDITOR_END_DISABLE_IF_RUNNING;

	if (ImGui::BeginPopup(POPUP_ADD_ENTITY)) {
		for (const EditorCreatableEntity& entity : m_CreatableEntities) {
			DrawCreatableEntityNode(entity);
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			import_entity(m_Context->get_registry(), std::filesystem::path(filePathName));
		}

		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

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

	entt::basic_view view = m_Context->get_registry().view<Relation>();

	auto tagComponent = m_Context->get_registry().try_get<TagComponent>((entt::entity) entity);
	std::string tag;
	if (tagComponent == nullptr) {
		tag = "Object #" + std::to_string(entity);
	} else {
		tag = ((TagComponent*) tagComponent)->m_Tag;
	}

	ImGuiTreeNodeFlags treeNodeFlags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	auto* children = m_Context->get_registry().try_get<Childs>((entt::entity) entity);
	bool hasChildren = children && children->m_childs.size() > 0;
	if (hasChildren) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isOpened = ImGui::TreeNodeEx((void*) (uint64_t) (uint32_t) entity, treeNodeFlags, tag.c_str());

	if (ImGui::IsItemClicked()) {
		StopRenaming();
		SelectOneNotify(entity);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		ImGui::SetDragDropPayload("Editor_SceneHierarchy_Payload", &entity, sizeof(ENTT_ID_TYPE));
		ImGui::Text(tag.c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Editor_SceneHierarchy_Payload")) {
			IM_ASSERT(payload->DataSize == sizeof(ENTT_ID_TYPE));
			entt::entity payloadEntity = *(const entt::entity*) payload->Data;
			entt::entity newParent = (entt::entity) entity;

#if _DEBUG
			printf("\nPAYLOAD: \n");
			printf(std::to_string((ENTT_ID_TYPE) payloadEntity).c_str());
			printf("\nENTITY: \n");
			printf(std::to_string((ENTT_ID_TYPE) newParent).c_str());
			printf("\n");
#endif

			if (!HasChild(payloadEntity, newParent)) {
				if (view.contains(payloadEntity)) {
					rebind_entity(m_Context->get_registry(), payloadEntity, newParent);
				} else {
					m_Context->get_registry().emplace<Relation>(payloadEntity, newParent);
				}
			}
		}
		ImGui::EndDragDropTarget();
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

		if (view.contains((entt::entity) entity) && ImGui::MenuItem("Ungroup")) {
			m_UngroupEntity = (entt::entity) entity;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, Constants::ERROR_COLOR);
		if (ImGui::MenuItem("Delete")) {
			isEntityDeleted = true;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	if (isOpened && hasChildren) {
		for (const auto& child : children->m_childs) {
			DrawEntityNode((ENTT_ID_TYPE) child);
		}

		ImGui::TreePop();
	}

	if (isEntityDeleted) {
		if (m_SelectionContext == entity)
			ResetSelectionNotify();
		if (m_Context->get_registry().valid((entt::entity) entity)) {
			m_Context->get_registry().destroy((entt::entity) entity);
		}
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
		entt::entity created = entt::null;
		switch (entity.Type) {
			case EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_CUBE:
				created = create_cube_entity_unlit(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::PRIMITIVE_PLANE:
				created = create_plane_entity_lit(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::LIGHT_DIRECTIONAL: {
				created = create_directional_light_entity(m_Context->get_registry(), glm::vec3(0.0f, 0.0f, 3.0f));
				break;
			}
			case EditorCreatableEntity::EditorCreatableEntityType::LIGHT_POINT: {
				created = create_point_light_entity(m_Context->get_registry(), glm::vec3(0.0f, 0.0f, 3.0f));
				break;
			}
			case EditorCreatableEntity::EditorCreatableEntityType::DEBUG_CUBE:
				created = create_debug_cube_entity(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::DEBUG_SPHERE:
				created = create_debug_sphere_entity(m_Context->get_registry());
				break;
			case EditorCreatableEntity::EditorCreatableEntityType::IMPORTABLE: {
				// Always center this window when appearing.
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				ImGui::SetNextWindowSize({400.f, 600.f});
				ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", "Choose Model", ".fbx,.obj", ".");
				break;
			}
		}

		if (m_Context->get_registry().valid(created)) {
			std::string title =
				std::string(entity.Title) + std::string(" #") + std::to_string((ENTT_ID_TYPE) created);
			m_Context->get_registry().emplace_or_replace<diffusion::TagComponent>(created, title);
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
	m_SingleDispatcher->dispatch({SceneInteractType::SELECTED_ONE, entity});
}

void Editor::SceneHierarchy::ResetSelectionNotify() {
	m_SelectionContext = ENTT_ID_TYPE(-1);
	IM_ASSERT(&m_SingleDispatcher != nullptr);
	m_SingleDispatcher->dispatch({SceneInteractType::RESET_SELECTION});
}

bool Editor::SceneHierarchy::HasChild(const entt::entity& origin, const entt::entity& target) {
	auto* children = m_Context->get_registry().try_get<Childs>((entt::entity) origin);
	bool hasChildren = children && children->m_childs.size() > 0;
	if (!hasChildren) {
		return false;
	}

	if (children->m_childs.contains(target)) {
		return true;
	}

	for (const entt::entity& child : children->m_childs) {
		bool result = HasChild(child, target);
		if (result) {
			return true;
		}
	}
	return false;
}

void Editor::SceneHierarchy::InitContexed() {
	Editor::GameWidget::InitContexed();

	m_SelectionContext = -1;
	m_IsRenameInputFocused = false;
	m_IsRenaming = false;
}
