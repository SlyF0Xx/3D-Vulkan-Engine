#include "BaseWidget.h"

void Editor::Widget::Render() {
	this->Render(0, 0);
}

void Editor::Widget::InitContexed() {
	if (m_IsInitContexed) {
		return;
	}
	m_IsInitContexed = true;
}
