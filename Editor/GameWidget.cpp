#include "GameWidget.h"

Editor::GameWidget::GameWidget(const diffusion::Ref<Game>& ctx) {
	SetContext(ctx);
}

void Editor::GameWidget::SetContext(const diffusion::Ref<Game>& game) {
	m_Context = game;
}
