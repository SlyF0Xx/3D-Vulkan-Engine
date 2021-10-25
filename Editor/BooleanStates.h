#pragma once

namespace Editor {

	void RevertBoolState(bool& p_state) {
		p_state = !p_state;
	}

	struct WindowStates {
		bool isContentBrowserOpen = true;
	};

}