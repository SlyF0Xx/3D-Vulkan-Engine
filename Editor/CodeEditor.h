#pragma once

#include <BaseComponents/TagComponent.h>

#include <entt/entt.hpp>

#include "widgets/TextEditor.h"
#include "GameWidget.h"
#include "GameProject.h"

namespace Editor {

	class CodeEditor : public GameWidget {
	public:
		CodeEditor() = delete;
		CodeEditor(entt::entity entity, EDITOR_GAME_TYPE ctx);
		void SetContext(EDITOR_GAME_TYPE ctx);
		void SetSelection(entt::entity entity);
		void Save();
		std::string GetTitle() const;
		ImGuiID GetID() const;
		void Render(bool* p_open, ImGuiWindowFlags flags) override;
	private:
		entt::entity m_Selection = entt::null;

#pragma region Widgets.
		TextEditor m_TextEditor;
#pragma endregion
	};

}
