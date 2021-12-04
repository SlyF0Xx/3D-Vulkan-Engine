#pragma once

#include <eventpp/eventdispatcher.h>

#include <entt/entt.hpp>

#include "Core/Base.h"

namespace Editor {

	enum class SceneInteractType {
		UNDEFINED,
		SELECTED_ONE,
		RESET_SELECTION,
		SELECTED_MANY,
	};

	struct SceneInteractEvent {
		SceneInteractType Type;
		int Length;
		ENTT_ID_TYPE* Entities;

		SceneInteractEvent() {
			Type = SceneInteractType::UNDEFINED;
			Length = 0;
			Entities = nullptr;
		}

		SceneInteractEvent(SceneInteractType type) {
			Length = 0;
			Type = type;
			Entities = nullptr;
		}

		SceneInteractEvent(SceneInteractType type, const ENTT_ID_TYPE& entity) {
			Length = 1;
			Type = type;
			Entities = new ENTT_ID_TYPE[Length] { entity };
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

	class SceneInteractionSingleTon {
	public:
		static SceneEventDispatcher GetDispatcher();
	private:
		static SceneEventDispatcher s_Dispatcher;
	};

}
