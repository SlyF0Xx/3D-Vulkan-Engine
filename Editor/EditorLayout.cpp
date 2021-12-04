#include "EditorLayout.h"

Editor::EditorLayout::EditorLayout(const diffusion::Ref<Game>& game) {
	SetContext(game);
}

void Editor::EditorLayout::SetContext(const diffusion::Ref<Game>& game) {
	m_Context = game;
}

Editor::EditorWindow* Editor::EditorLayout::GetParent() {
	return m_Parent;
}
