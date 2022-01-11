#pragma once

#include "GroupPanel.h"
#include "GameWidget.h"
#include "GameProject.h"

#include <BTLib.h>

namespace Editor {

	struct EditorCreatableBehaviourTreeNode {
		enum class EditorCreatableBehaviourTreeType {
			SEQUENCE,
			SELECTOR,
			PARALLEL,

			DEC_INVERT,
			DEC_REPEAT_N,
			DEC_RETRY_N,
			DEC_FORCE_SUCCEED,
			DEC_COOLDOWN,

			ACTION,
			CONDITION
		};

		const char* Title;
		EditorCreatableBehaviourTreeType Type = EditorCreatableBehaviourTreeType::SEQUENCE;
		EditorCreatableBehaviourTreeNode* Children = nullptr;
		uint32_t Size = 0;

		EditorCreatableBehaviourTreeNode(const char* T, EditorCreatableBehaviourTreeType Ty) : Title(T), Type(Ty), Size(0) {};
		EditorCreatableBehaviourTreeNode(const char* T, EditorCreatableBehaviourTreeNode* C, uint32_t S) : Title(T), Children(C), Size(S) {};
	};

	class BehaviourTreeEditor : public GameWidget {
	public:
		BehaviourTreeEditor() = delete;
		explicit BehaviourTreeEditor(EDITOR_GAME_TYPE ctx);

		virtual void Render(bool* p_open, ImGuiWindowFlags flags) override;

		void SetSelection(entt::entity entity);

	private:
		void RenderEditor();

		void DrawBehaviourTreeNode(Behaviour& node);

		std::string GetTitle(Behaviour& node);

		std::vector<Behaviour>* GetChildren(Behaviour& node);
		
		Behaviour* FindParent(Behaviour& from, Behaviour& target);

		void DeleteBehaviourTreeNode(Behaviour& from, Behaviour& target);

		void DrawCreatableNode(EditorCreatableBehaviourTreeNode node);

	private:
		entt::entity m_Selection = entt::null;
		BTComponent* m_BehaviourTreeComponent = nullptr;

		bool m_IsBehaviourTreeEdit = false;
		bool m_IsBehaviourTreeInit = false;
// 		Behaviour m_SelectedLeaf;
		Behaviour* m_BehaviourSelection = nullptr;

		char m_InitBuf[Constants::ACTOR_NAME_LENGTH] = "";
		char m_FinishBuf[Constants::ACTOR_NAME_LENGTH] = "";
		char m_TickBuf[Constants::ACTOR_NAME_LENGTH] = "";
		char m_AbortBuf[Constants::ACTOR_NAME_LENGTH] = "";
		char m_CondBuf[Constants::ACTOR_NAME_LENGTH] = "";

		const static inline EditorCreatableBehaviourTreeNode s_CreatableNodes[4] = {
			EditorCreatableBehaviourTreeNode("Run",
				new EditorCreatableBehaviourTreeNode[3] {
					EditorCreatableBehaviourTreeNode("Selector", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::SELECTOR),
					EditorCreatableBehaviourTreeNode("Sequence", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::SEQUENCE),
					EditorCreatableBehaviourTreeNode("Parallel", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::PARALLEL),
				},
			3),
			EditorCreatableBehaviourTreeNode("Action", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::ACTION),
			EditorCreatableBehaviourTreeNode("Condition", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::CONDITION),
			EditorCreatableBehaviourTreeNode("Decorators",
				new EditorCreatableBehaviourTreeNode[5] {
					EditorCreatableBehaviourTreeNode("Negative", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_INVERT),
					EditorCreatableBehaviourTreeNode("Repeat N Times", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_REPEAT_N),
					EditorCreatableBehaviourTreeNode("Retry N Times", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_RETRY_N),
					EditorCreatableBehaviourTreeNode("Force Succeed", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_FORCE_SUCCEED),
					EditorCreatableBehaviourTreeNode("Cooldown Timer", EditorCreatableBehaviourTreeNode::EditorCreatableBehaviourTreeType::DEC_COOLDOWN),
				},
			5),
		};
	};

}
