#include "ViewportSnapInteraction.h"

// LNK2001.
Editor::ViewportEventDispatcher Editor::ViewportSnapInteractionSingleTon::s_Dispatcher;

Editor::ViewportEventDispatcher Editor::ViewportSnapInteractionSingleTon::GetDispatcher() {
	if (s_Dispatcher) {
		return s_Dispatcher;
	}
	s_Dispatcher = diffusion::CreateRef<ViewportEventDispatcherSrc>();
	return GetDispatcher();
}
