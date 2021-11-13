#pragma once

#include <stdint.h>

namespace Editor {

	namespace Constants {

#ifndef EDITOR_CONST
#define EDITOR_CONST static inline constexpr const
#endif

#ifndef EDITOR_CONST_UINT
#define EDITOR_CONST_UINT EDITOR_CONST uint32_t
#endif

		/// <summary>
		/// Length of actor's name. Uses in SceneHierarchy, TagComponentInspector.
		/// </summary>
		EDITOR_CONST_UINT ACTOR_NAME_LENGTH = 64;

	}

}