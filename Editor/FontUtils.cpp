#include "FontUtils.h"

// LNK2001.
std::map<Editor::FONT_TYPE, ImFont*> Editor::FontUtils::s_Fonts;

ImFont* Editor::FontUtils::GetFont(Editor::FONT_TYPE type) {
	if (s_Fonts.count(type) > 0) {
		return s_Fonts[type];
	}
	return nullptr;
}

void Editor::FontUtils::ReleaseFonts() {
	s_Fonts.clear();
}

void Editor::FontUtils::LoadFonts() {
	ImGuiIO& io = ImGui::GetIO();
	if (GetFont(FONT_TYPE::PRIMARY_TEXT) == NULL) {
		s_Fonts[FONT_TYPE::PRIMARY_TEXT] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Roboto-Medium.ttf", 16.0f);
		IM_ASSERT(GetFont(FONT_TYPE::PRIMARY_TEXT) != NULL);
	}

	if (GetFont(FONT_TYPE::SUBHEADER_TEXT) == NULL) {
		s_Fonts[FONT_TYPE::SUBHEADER_TEXT] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Roboto-Medium.ttf", 12.0f);
		IM_ASSERT(GetFont(FONT_TYPE::SUBHEADER_TEXT) != NULL);
	}

	if (GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY) == NULL) {
		s_Fonts[FONT_TYPE::LUA_EDITOR_PRIMARY] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Droid-Sans-Mono.ttf", 24.0f);
		IM_ASSERT(GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY) != NULL);
	}
}
