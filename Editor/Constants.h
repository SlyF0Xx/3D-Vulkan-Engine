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

#pragma region Colors.
		EDITOR_CONST ImU32 OVERLAY_DEFAULT_COLOR = IM_COL32(110, 110, 110, 160);

		EDITOR_CONST ImU32 OVERLAY_SUCCESS_COLOR = IM_COL32(60, 189, 112, 160);
		EDITOR_CONST ImU32 OVERLAY_HOVER_COLOR = IM_COL32(51, 162, 96, 190);
		EDITOR_CONST ImU32 OVERLAY_ACTIVE_COLOR = IM_COL32(47, 147, 87, 190);

		EDITOR_CONST ImU32 SUCCESS_COLOR = IM_COL32(60, 189, 112, 160);
		EDITOR_CONST ImU32 HOVER_COLOR = IM_COL32(51, 162, 96, 190);
		EDITOR_CONST ImU32 ACTIVE_COLOR = IM_COL32(47, 147, 87, 190);
#pragma endregion

	}

}