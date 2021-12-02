#include "SceneInteraction.h"

// LNK2001.
Editor::SceneEventDispatcher Editor::SceneInteractionSingleTon::s_Dispatcher;

Editor::SceneEventDispatcher Editor::SceneInteractionSingleTon::GetDispatcher() {
	if (s_Dispatcher) {
		return s_Dispatcher;
	}
	s_Dispatcher = diffusion::CreateRef<SceneEventDispatcherSrc>();
	return GetDispatcher();
}
