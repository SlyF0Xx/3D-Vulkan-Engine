#pragma once

#include <stdint.h>

namespace Editor {

	namespace Constants {

#ifndef EDITOR_CONSTEXPR
#define EDITOR_CONSTEXPR static inline constexpr const
#endif

#ifndef EDITOR_CONST
#define EDITOR_CONST static inline const
#endif

#ifndef EDITOR_CONST_UINT
#define EDITOR_CONST_UINT EDITOR_CONSTEXPR uint32_t
#endif

#ifndef EDITOR_CONST_IM_VEC2
#define EDITOR_CONST_IM_VEC2 EDITOR_CONST ImVec2
#endif

		/// <summary>
		/// Length of actor's name. Uses in SceneHierarchy, TagComponentInspector.
		/// </summary>
		EDITOR_CONST_UINT ACTOR_NAME_LENGTH = 64;

		EDITOR_CONST_IM_VEC2 EDITOR_WINDOW_PADDING = {8.f, 8.f};

	}

}