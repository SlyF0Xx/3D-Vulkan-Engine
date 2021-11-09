#pragma once

#include <eventpp/eventdispatcher.h>

#include <entt/entt.hpp>

#include "Core/Base.h"

namespace Editor {

	enum class SceneInteractType {
		SELECTED_ONE,
		RESET_SELECTION,
		SELECTED_MANY,
	};

	struct SceneInteractEvent {
		SceneInteractType Type;
		int Length;
		ENTT_ID_TYPE* Entities;

		SceneInteractEvent(SceneInteractType type) {
			Length = 0;
			Type = type;
			Entities = nullptr;
		}

		SceneInteractEvent(SceneInteractType type, const ENTT_ID_TYPE& entity) {
			ENTT_ID_TYPE e = entity;

			Length = 1;
			Type = type;
			Entities = &e;
		}
	};

	struct SceneInteractEventPolicies {
		static SceneInteractType getEvent(const SceneInteractEvent& e) {
			return e.Type;
		}
	};

	typedef void SceneInteractFunc(const SceneInteractEvent&);
	using SceneEventDispatcherSrc	= eventpp::EventDispatcher<SceneInteractType, SceneInteractFunc, SceneInteractEventPolicies>;
	using SceneEventDispatcher		= diffusion::Ref<SceneEventDispatcherSrc>;

}
