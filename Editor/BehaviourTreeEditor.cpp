#include "BehaviourTreeEditor.h"

Editor::BehaviourTreeEditor::BehaviourTreeEditor(EDITOR_GAME_TYPE ctx)
	: Editor::GameWidget(ctx) {
	// ..
}

void Editor::BehaviourTreeEditor::Render(bool* p_open, ImGuiWindowFlags flags) {
	EDITOR_BEGIN_DISABLE_IF_RUNNING;
	ImGui::BeginGroupPanel("Behaviour Tree", ImVec2(-1.0f, -1.0f));
	bool isBTExists = !!m_BehaviourTreeComponent;

	if (!isBTExists) {
		if (ImGui::Button("Create")) {
			m_BehaviourTreeComponent = &m_Context->get_registry().emplace<BTComponent>(m_Selection);
		}
	} else {
		if (ImGui::Button("Modify##BehviourTree")) {
			// Always center this window when appearing.
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(viewport->Size * 0.8f);

			m_IsBehaviourTreeInit = false;
			m_IsBehaviourTreeEdit = true;
		}
	}
	ImGui::SameLine();

	if (!isBTExists || true) ImGui::BeginDisabled();
	if (ImGui::Button("Export")) {
		// TODO:
	}
	ImGui::SameLine();

	if (ImGui::Button("Import")) {
		// TODO:
	}
	ImGui::SameLine();
	if (!isBTExists || true) ImGui::EndDisabled();

	if (!isBTExists || m_IsBehaviourTreeEdit) ImGui::BeginDisabled();
	if (ImGui::Button("Remove")) {
		// TODO: Add confirmation!
		m_Context->get_registry().remove<BTComponent>(m_Selection);
		m_BehaviourTreeComponent = nullptr;
	}
	if (!isBTExists || m_IsBehaviourTreeEdit) ImGui::EndDisabled();
	ImGui::EndGroupPanel();

	if (m_IsBehaviourTreeEdit) {
		RenderEditor();
	}
	EDITOR_END_DISABLE_IF_RUNNING;
}

void Editor::BehaviourTreeEditor::SetSelection(entt::entity entity) {
	m_Selection = entity;
	m_IsBehaviourTreeEdit = false;
	m_IsBehaviourTreeInit = false;
	if (m_Selection != entt::null && m_Context->get_registry().valid(m_Selection)) {
		m_BehaviourTreeComponent = m_Context->get_registry().try_get<BTComponent>(m_Selection);		
	} else {
		m_BehaviourTreeComponent = nullptr;
	}
}

void Editor::BehaviourTreeEditor::RenderEditor() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.f);

	ImGui::Begin("Behaviour Tree Editor", &m_IsBehaviourTreeEdit, ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

	//ImGui::BeginChild("DockSpace##BT");
	ImGuiID id = ImGui::GetID("Dockspace##BT");
	ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), 0);

	if (!m_IsBehaviourTreeInit) {

		// Docks building.
		ImGui::DockBuilderRemoveNode(id); // clear any previous layout
		ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace);
		//ImGui::DockBuilderSetNodeSize(m_DockIDs.MainDock, viewport->Size);

		// Order important!
		ImGuiID r = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 1.f, nullptr, &id);

		ImGui::DockBuilderDockWindow("Inspector##BT", r);
		ImGui::DockBuilderDockWindow("Tree##BT", id);
		ImGui::DockBuilderFinish(id);

		m_IsBehaviourTreeInit = true;
	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	ImGui::SetNextWindowDockID(id, ImGuiCond_FirstUseEver);
	ImGui::Begin("Tree##BT");
	DrawBehaviourTreeNode(m_BehaviourTreeComponent->root);
	ImGui::End(); // Tree.

	//ImGui::SetNextWindowDockID(r, ImGuiCond_FirstUseEver);
	ImGui::Begin("Inspector##BT");

	if (m_BehaviourSelection) {
		switch (m_BehaviourSelection->index()) {
			case 2: { // Parallel.
				ImGui::BeginGroupPanel("Parallel", ImVec2(-1.0f, -1.f));

				Parallel& parallel = std::get<Parallel>(*m_BehaviourSelection);

				static std::map<ParallelPolicy, std::string> policies;
				policies[ParallelPolicy::FailAll] = "Fail All";
				policies[ParallelPolicy::FailAny] = "Fail Any";
				policies[ParallelPolicy::RequireAll] = "Require All";
				policies[ParallelPolicy::RequireAny] = "Require Any";
				policies[ParallelPolicy::SuccessAll] = "Success All";
				policies[ParallelPolicy::SuccessAny] = "Success Any";

				if (ImGui::BeginCombo("Policy", policies[parallel.policy].c_str(), 0)) {
					for (auto it = policies.begin(); it != policies.end(); it++) {
						const bool is_selected = (parallel.policy == it->first);
						if (ImGui::Selectable(it->second.c_str(), is_selected))
							parallel.policy = it->first;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::EndGroupPanel();
				break;
			}
			case 3: // InvertDecorator.
			case 6: { // ForceSucceededDecorator.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " could contain only one child.").c_str());
				ImGui::EndGroupPanel();
				break;
			}
			case 4: {	// RepeatNDecorator.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " could contain only one child.").c_str());
				int i = static_cast<int>(std::get<RepeatNDecorator>(*m_BehaviourSelection).N);
				if (ImGui::SliderInt("N", &i, 1, 100)) {
					std::get<RepeatNDecorator>(*m_BehaviourSelection).N = i;
				}
				ImGui::EndGroupPanel();
				break;
			}
			case 5: {	// RetryNDecorator.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " could contain only one child.").c_str());
				int i = static_cast<int>(std::get<RetryNDecorator>(*m_BehaviourSelection).N);
				if (ImGui::DragInt("N", &i, 1, 100)) {
					std::get<RetryNDecorator>(*m_BehaviourSelection).N = i;
				}
				ImGui::EndGroupPanel();
				break;
			}
			case 7: {// CooldownDecorator.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " could contain only one child.").c_str());
				float i = static_cast<float>(std::get<CooldownDecorator>(*m_BehaviourSelection).capacity);
				if (ImGui::DragFloat("Capacity", &i, 1.f, 1.f, 360.f, "%.2f", 1.f)) {
					std::get<CooldownDecorator>(*m_BehaviourSelection).capacity = i;
				}
				ImGui::EndGroupPanel();
				break;
			}
			case 8: { // BehaviourAction.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " couldn't contain children.").c_str());

				BehaviourAction& action = std::get<BehaviourAction>(*m_BehaviourSelection);

				ImGui::InputText("onInit", m_InitBuf, Constants::ACTOR_NAME_LENGTH, 0);
				ImGui::InputText("onTick", m_TickBuf, Constants::ACTOR_NAME_LENGTH, 0);
				ImGui::InputText("onFinish", m_FinishBuf, Constants::ACTOR_NAME_LENGTH, 0);
				ImGui::InputText("onAbort", m_AbortBuf, Constants::ACTOR_NAME_LENGTH, 0);

				if (ImGui::Button("Apply Changes")) {
					action.ActionOnInit = std::string(m_InitBuf);
					action.ActionOnTick = std::string(m_TickBuf);
					action.ActionOnFinish = std::string(m_FinishBuf);
					action.ActionOnAbort = std::string(m_AbortBuf);
				}

				ImGui::EndGroupPanel();
				break;
			}
			case 9: { // BehaviourCondition.
				std::string title = GetTitle(*m_BehaviourSelection);
				ImGui::BeginGroupPanel(title.c_str(), ImVec2(-1.0f, -1.f));
				ImGui::Text((title + " couldn't contain children.").c_str());

				ImGui::InputText("Condition Function", m_CondBuf, Constants::ACTOR_NAME_LENGTH, 0);

				if (ImGui::Button("Apply Changes")) {
					std::get<BehaviourCondition>(*m_BehaviourSelection).ConditionName = std::string(m_CondBuf);
				}

				ImGui::EndGroupPanel();
				break;
			}
		}
	}

	ImGui::BeginGroupPanel("General", ImVec2(-1.0f, -1.f));
	if (ImGui::Button("Add Behaviour Tree Node", ImVec2(-1.0f, 40.0f))) {
		ImGui::OpenPopup("Add Node");
	}

	if (ImGui::BeginPopup("Add Node")) {
		for (const EditorCreatableBehaviourTreeNode& v : BehaviourTreeEditor::s_CreatableNodes) {
			DrawCreatableNode(v);
		}
		ImGui::EndPopup();
	}
	ImGui::EndGroupPanel();
	ImGui::End(); // Inspector.
}

void Editor::BehaviourTreeEditor::DrawBehaviourTreeNode(Behaviour& node) {
	ImGuiTreeNodeFlags treeNodeFlags = (m_BehaviourSelection == &node) ? ImGuiTreeNodeFlags_Selected : 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	std::string title = GetTitle(node);
	std::vector<Behaviour>* children = GetChildren(node);

	bool hasChildren = !!children && children->size() > 0;

	if (hasChildren) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isOpened = ImGui::TreeNodeEx((void *) &node, treeNodeFlags, title.c_str());

	if (ImGui::IsItemClicked()) {
		m_BehaviourSelection = &node;

		switch (node.index()) {
			case 8: { // BehaviourAction.
				BehaviourAction& action = std::get<BehaviourAction>(node);
				strcpy_s(m_InitBuf, Constants::ACTOR_NAME_LENGTH, action.ActionOnInit.c_str());
				strcpy_s(m_FinishBuf, Constants::ACTOR_NAME_LENGTH, action.ActionOnFinish.c_str());
				strcpy_s(m_TickBuf, Constants::ACTOR_NAME_LENGTH, action.ActionOnTick.c_str());
				strcpy_s(m_AbortBuf, Constants::ACTOR_NAME_LENGTH, action.ActionOnAbort.c_str());
				break;
			}
			case 9: { // BehaviourCondition.
				BehaviourCondition& condition = std::get<BehaviourCondition>(node);
				strcpy_s(m_CondBuf, Constants::ACTOR_NAME_LENGTH, condition.ConditionName.c_str());
				break;
			}
		}

		
	}

	bool isNodeDeleted = false;
	if (ImGui::BeginPopupContextItem()) {
		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Actions");
		ImGui::PopFont();
		ImGui::Separator();

		if (ImGui::MenuItem("Reset Selection (Jump to Root)")) {
			m_BehaviourSelection = nullptr;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, Constants::ERROR_COLOR);
		if (ImGui::MenuItem("Delete")) {
			isNodeDeleted = true;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	if (isOpened && hasChildren) {
		for (auto& child : *children) {
			DrawBehaviourTreeNode(child);
		}

		ImGui::TreePop();
	}

	if (isNodeDeleted) {
		if (m_BehaviourSelection == &node)
			m_BehaviourSelection = nullptr;
		DeleteBehaviourTreeNode(m_BehaviourTreeComponent->root, node);
	}
}

std::string Editor::BehaviourTreeEditor::GetTitle(Behaviour& node) {
	switch (node.index()) {
		case 0: // Sequence.
			return "Sequence";
		case 1: // Selector.
			return "Selector";
		case 2: // Parallel.
			return "Parallel";
		case 3: // InvertDecorator.
			return "Invert";
		case 4: // RepeatNDecorator.
			return "Repeat N";
		case 5: // RetryNDecorator.
			return "Retry N";
		case 6: // ForceSucceededDecorator.
			return "Force Succeeded";
		case 7: // CooldownDecorator.
			return "Cooldown";
		case 8: // BehaviourAction.
			return "Action";
		case 9: // BehaviourCondition.
			return "Condition";
	}
	return "Unknown Node";
}

std::vector<Behaviour>* Editor::BehaviourTreeEditor::GetChildren(Behaviour& node) {
	switch (node.index()) {
		case 0: // Sequence.
			return &std::get<Sequence>(node).childs;
		case 1: // Selector.
			return &std::get<Selector>(node).childs;
		case 2: // Parallel.
			return &std::get<Parallel>(node).childs;
		case 3: // InvertDecorator.
			return &std::get<InvertDecorator>(node).child;
		case 4: // RepeatNDecorator.
			return &std::get<RepeatNDecorator>(node).child;
		case 5: // RetryNDecorator.
			return &std::get<RetryNDecorator>(node).child;
		case 6: // ForceSucceededDecorator.
			return &std::get<ForceSucceededDecorator>(node).child;
		case 7: // CooldownDecorator.
			return &std::get<CooldownDecorator>(node).child;
		case 8: // BehaviourAction.
		case 9: // BehaviourCondition.
			return nullptr;
	}
	return nullptr;
}

Behaviour* Editor::BehaviourTreeEditor::FindParent(Behaviour& from, Behaviour& target) {
	if (&from == &target) {
		return nullptr;
	}

	std::vector<Behaviour>* children = GetChildren(from);
	if (!children || children->size() <= 0) {
		return nullptr;
	}

	for (auto& child : *children) {
		if (&child == &target) {
			return &from;
		} 
		auto ptr = FindParent(child, target);
		if (ptr) {
			return ptr;
		}
	}
	return nullptr;
}

void Editor::BehaviourTreeEditor::DeleteBehaviourTreeNode(Behaviour& from, Behaviour& target) {
	if (&m_BehaviourTreeComponent->root == &target) {
		m_BehaviourTreeComponent->root = Selector {};
		return;
	}

	Behaviour* parent = FindParent(from, target);
	if (parent) {
		std::vector<Behaviour>* children = GetChildren(*parent);
		if (children) {
			auto it = std::find_if(children->begin(), children->end(), [&](const Behaviour& bh) {
				return &bh == &target;
			});

			if (it != children->end()) {
				children->erase(it);
			}
		}
	}
}

void Editor::BehaviourTreeEditor::DrawCreatableNode(EditorCreatableBehaviourTreeNode node) {
	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

	if (node.Children) {
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
	} else {
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool isLocked = false;
	if (m_BehaviourSelection) {
		switch (m_BehaviourSelection->index()) {
			case 3: // InvertDecorator.
				isLocked = std::get<InvertDecorator>(*m_BehaviourSelection).child.size() > 0;
				break;
			case 4: // RepeatNDecorator.
				isLocked = std::get<RepeatNDecorator>(*m_BehaviourSelection).child.size() > 0;
				break;
			case 5: // RetryNDecorator.
				isLocked = std::get<RetryNDecorator>(*m_BehaviourSelection).child.size() > 0;
				break;
			case 6: // ForceSucceededDecorator.
				isLocked = std::get<ForceSucceededDecorator>(*m_BehaviourSelection).child.size() > 0;
				break;
			case 7: // CooldownDecorator.
				isLocked = std::get<CooldownDecorator>(*m_BehaviourSelection).child.size() > 0;
				break;
			case 8: // BehaviourAction.
			case 9: // BehaviourCondition.
				isLocked = true;
		}
	}

	if (isLocked) ImGui::BeginDisabled();

	bool isOpened = ImGui::TreeNodeEx(node.Title, treeNodeFlags);

	if (ImGui::IsItemClicked() && !node.Children) {
		Behaviour what;
		switch (node.Type) {
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::SELECTOR:
				what = Selector {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::SEQUENCE:
				what = Sequence {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::PARALLEL:
				what = Parallel {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_INVERT:
				what = InvertDecorator {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_REPEAT_N:
				what = RepeatNDecorator {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_RETRY_N:
				what = RetryNDecorator {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_FORCE_SUCCEED:
				what = ForceSucceededDecorator {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_COOLDOWN:
				what = CooldownDecorator {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::ACTION:
				what = BehaviourAction {};
				break;
			case EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::CONDITION:
				what = BehaviourCondition {};
				break;
		}

		if (!m_BehaviourSelection) {
			m_BehaviourTreeComponent->root = what;
		} else {
			switch (m_BehaviourSelection->index()) {
				case 0: // Sequence.
					std::get<Sequence>(*m_BehaviourSelection).childs.push_back(what);
					break;
				case 1: // Selector.
					std::get<Selector>(*m_BehaviourSelection).childs.push_back(what);
					break;
				case 2: // Parallel.
					std::get<Parallel>(*m_BehaviourSelection).childs.push_back(what);
					break;
				case 3: // InvertDecorator.
					std::get<InvertDecorator>(*m_BehaviourSelection).child.push_back(what);
					break;
				case 4: // RepeatNDecorator.
					std::get<RepeatNDecorator>(*m_BehaviourSelection).child.push_back(what);
					break;
				case 5: // RetryNDecorator.
					std::get<RetryNDecorator>(*m_BehaviourSelection).child.push_back(what);
					break;
				case 6: // ForceSucceededDecorator.
					std::get<ForceSucceededDecorator>(*m_BehaviourSelection).child.push_back(what);
					break;
				case 7: // CooldownDecorator.
					std::get<CooldownDecorator>(*m_BehaviourSelection).child.push_back(what);
					break;
				case 8: // BehaviourAction.
				case 9: // BehaviourCondition.
					throw;
			}
		}

		ImGui::CloseCurrentPopup();
	}

	if (isOpened && node.Children) {
		for (auto i = 0; i < node.Size; i++) {
			DrawCreatableNode(node.Children[i]);
		}

		ImGui::TreePop();
	}

	if (isLocked) ImGui::EndDisabled();
}
