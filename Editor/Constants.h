#pragma once

#include <stdint.h>

namespace Editor {

#ifndef EDITOR_GAME_TYPE
#define EDITOR_GAME_TYPE Game*
#endif

#ifndef EDITOR_BEGIN_DISABLE_IF_RUNNING
#define EDITOR_BEGIN_DISABLE_IF_RUNNING if (GameProject::Instance()->IsRunning()) { ImGui::BeginDisabled(); }
#endif

#ifndef EDITOR_END_DISABLE_IF_RUNNING
#define EDITOR_END_DISABLE_IF_RUNNING if (GameProject::Instance()->IsRunning()) { ImGui::EndDisabled(); }
#endif

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
		EDITOR_CONST_UINT ACTOR_NAME_LENGTH			= 64;

		EDITOR_CONST_IM_VEC2 EDITOR_WINDOW_PADDING	= {8.f, 8.f};

#pragma region Colors.
		EDITOR_CONST ImU32 OVERLAY_DEFAULT_COLOR	= IM_COL32(110, 110, 110, 190);

		EDITOR_CONST ImU32 OVERLAY_SUCCESS_COLOR	= IM_COL32(60, 189, 112, 190);
		EDITOR_CONST ImU32 OVERLAY_HOVER_COLOR		= IM_COL32(51, 162, 96, 220);
		EDITOR_CONST ImU32 OVERLAY_ACTIVE_COLOR		= IM_COL32(47, 147, 87, 220);

		EDITOR_CONST ImU32 SUCCESS_COLOR			= IM_COL32(60, 189, 112, 190);
		EDITOR_CONST ImU32 HOVER_COLOR				= IM_COL32(51, 162, 96, 220);
		EDITOR_CONST ImU32 ACTIVE_COLOR				= IM_COL32(47, 147, 87, 220);

		EDITOR_CONST ImU32 ERROR_COLOR				= IM_COL32(255, 52, 52, 255);
#pragma endregion

		EDITOR_CONST char* SCRIPT_TEMPLATE =
			"-- Available functions:\n"
			"-- global_translate(entity, x, y, z);\n"
			"-- local_translate(entity, x, y, z);\n"
			"-- local_rotate(entity, x, y, z);\n"
			"-- local_scale(entity, x, y, z);\n"
			"-- spawn_entity(); -- Create new entity.\n"
			"-- change_name(entity, name);\n"
			"-- add_transform(entity); -- Apply transform component to entity. Throw exception if component exists.\n"
			"-- import_mesh(entity, path);\n"
			"-- get_entity_by_name(name);\n"
			"-- destroy_entity(entity);\n"
			"-- \n"
			"-- Example:\n"
			"-- function on_trigger()\n"
			"--     tv = get_entity_by_name(\"Name\");\n"
			"--     local_translate(tv, 0, 0, 50);\n"
			"-- end\n";

	}

}