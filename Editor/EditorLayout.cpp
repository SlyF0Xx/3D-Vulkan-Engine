#include "EditorLayout.h"

Editor::EditorLayout::EditorLayout() {
	// TODO: Deprecated.
	m_Context = GameProject::Instance()->GetCurrentContext();
}

Editor::EditorWindow* Editor::EditorLayout::GetParent() {
	return m_Parent;
}
