#include "WindowTitleInteraction.h"

// LNK2001.
Editor::WindowTitleDispatcher Editor::WindowTitleInteractionSingleTon::s_Dispatcher;

Editor::WindowTitleDispatcher Editor::WindowTitleInteractionSingleTon::GetDispatcher() {
	if (s_Dispatcher) {
		return s_Dispatcher;
	}
	s_Dispatcher = diffusion::CreateRef<Editor::WindowTitleDispatcherSrc>();
	return GetDispatcher();
}
