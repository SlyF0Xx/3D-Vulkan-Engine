#pragma once

#include <eventpp/eventdispatcher.h>

#include <entt/entt.hpp>

#include "Core/Base.h"

namespace Editor {

	enum class WindowTitleInteractionType {
		TITLE_UPDATED,
		CONTEXT_CHANGED
	};

	using WindowTitleDispatcherSrc = eventpp::EventDispatcher<Editor::WindowTitleInteractionType, void (void)>;
	using WindowTitleDispatcher = diffusion::Ref<WindowTitleDispatcherSrc>;

	class WindowTitleInteractionSingleTon {
	public:
		static WindowTitleDispatcher GetDispatcher();
	private:
		static WindowTitleDispatcher s_Dispatcher;
	};

}
