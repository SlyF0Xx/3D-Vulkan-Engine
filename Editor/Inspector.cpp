#include "Inspector.h"

Editor::Inspector::Inspector(EDITOR_GAME_TYPE game)
	: GameWidget(game), 
	m_TagInspector(game), 
	m_TransformInspector(game), 
	m_ScriptInspector(game),
	m_PhysicsInspector(game) {
	m_SingleDispatcher = SceneInteractionSingleTon::GetDispatcher();

	IM_ASSERT(&m_SingleDispatcher != nullptr);

	m_SingleDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& sEvent) {
		m_Selection = (entt::entity) sEvent.Entities[0];
	});

	m_SingleDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& sEvent) {
		m_Selection = entt::null;
	});
}

void Editor::Inspector::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	ImGui::Begin("Inspector", p_open, flags);

	if (!m_Context->get_registry().valid(m_Selection)) {
		ImGui::Text("Nothing to inspect.");
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		return;
	}

	m_TagInspector.Render();
	m_TransformInspector.Render();
	m_ScriptInspector.Render();
	m_PhysicsInspector.Render();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Constants::EDITOR_WINDOW_PADDING);
	const char* text = "Add Component";
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	EDITOR_BEGIN_DISABLE_IF_RUNNING
	if (ImGui::Button(text, {0.f, 0.f})) {
		ImGui::OpenPopup(POPUP_ADD_COMPONENT);
	}
	EDITOR_END_DISABLE_IF_RUNNING

	if (ImGui::BeginPopup(POPUP_ADD_COMPONENT)) {
		for (const EditorCreatableComponent& entity : m_CreatableComponents) {
			DrawCreatableComponentNode(entity);
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void Editor::Inspector::SetContext(EDITOR_GAME_TYPE ctx) {
	Editor::GameWidget::SetContext(ctx);
	m_TagInspector.SetContext(ctx);
	m_TransformInspector.SetContext(ctx);
	m_ScriptInspector.SetContext(ctx);
	m_PhysicsInspector.SetContext(ctx);

	m_Selection = entt::null;
}

void Editor::Inspector::OnRegisterUpdated() {
	Editor::GameWidget::OnRegisterUpdated();
	m_TagInspector.OnRegisterUpdated();
	m_TransformInspector.OnRegisterUpdated();
	m_ScriptInspector.OnRegisterUpdated();
	m_PhysicsInspector.OnRegisterUpdated();
}

void Editor::Inspector::DrawCreatableComponentNode(const EditorCreatableComponent& comp) {
	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

	if (comp.Children) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isLocked = false;
	switch (comp.Type) {
		case EditorCreatableComponent::EditorCreatableComponentType::SCRIPT:
			isLocked = m_Context->get_registry().view<ScriptComponent>().contains(m_Selection);
			break;
		case EditorCreatableComponent::EditorCreatableComponentType::TRANSFORM:
			isLocked = m_Context->get_registry().view<TransformComponent>().contains(m_Selection);
			break;
		case EditorCreatableComponent::EditorCreatableComponentType::TAG:
			isLocked = m_Context->get_registry().view<TagComponent>().contains(m_Selection);
			break;
	}

	if (isLocked) {
		ImGui::BeginDisabled();
	}

	bool isOpened = ImGui::TreeNodeEx(comp.Title, treeNodeFlags);

	if (ImGui::IsItemClicked() && !comp.Children) {
		switch (comp.Type) {
			case EditorCreatableComponent::EditorCreatableComponentType::SCRIPT: {
				m_Context->get_registry().emplace<diffusion::ScriptComponent>(m_Selection, 
					std::string(Constants::SCRIPT_TEMPLATE));
				ScriptComponentState luaScript = m_Context->get_registry().get<ScriptComponentState>(m_Selection);
				LoadImguiBindings(luaScript.m_state);
				m_Context->get_registry().emplace_or_replace<ScriptComponentState>(m_Selection, luaScript.m_state);
			}
				break;
			case EditorCreatableComponent::EditorCreatableComponentType::TRANSFORM:
				m_Context->get_registry().emplace_or_replace<diffusion::TransformComponent>(m_Selection);
				break;
			case EditorCreatableComponent::EditorCreatableComponentType::TAG:
				m_Context->get_registry().emplace_or_replace<diffusion::TagComponent>(m_Selection, "Sample Title");
				break;
		}
		m_SingleDispatcher->dispatch({SceneInteractType::SELECTED_ONE, (ENTT_ID_TYPE) m_Selection});
		ImGui::CloseCurrentPopup();
	}

	if (isOpened && comp.Children) {
		for (auto i = 0; i < comp.Size; i++) {
			DrawCreatableComponentNode(comp.Children[i]);
		}

		ImGui::TreePop();
	}

	if (isLocked) {
		ImGui::EndDisabled();
	}
}
