#pragma once

#include "imgui.h"

#include <map>

namespace Editor {

	enum class FONT_TYPE {
		PRIMARY_TEXT,
		SUBHEADER_TEXT,
		LUA_EDITOR_PRIMARY
	};

	class FontUtils {
	public:
		static ImFont* GetFont(Editor::FONT_TYPE type);

		static void ReleaseFonts();

	private:
		static void LoadFonts();

	private:
		static std::map<FONT_TYPE, ImFont*> s_Fonts;

		friend class EditorWindow;
	};

}
