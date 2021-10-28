#pragma once

#include "imgui.h"

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}

#include <LuaBridge/LuaBridge.h>
#include <iostream>

using namespace luabridge;

class LuaConsole {
public:
	LuaConsole();
	~LuaConsole();

	void ClearLog();

	void AddLog(const char* fmt, ...) IM_FMTARGS(2);

	void Render();

	void Render(bool* p_open, ImGuiWindowFlags flags);

	void ExecCommand(const char* command_line);

	int TextEditCallback(ImGuiInputTextCallbackData* data);

	int LuaPrint(const std::string& s);

	// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
		LuaConsole* console = (LuaConsole*) data->UserData;
		return console->TextEditCallback(data);
	}

	// Portable helpers
	static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
	static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
	static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*) memcpy(buf, (const void*) s, len); }
	static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

private:
	static inline constexpr const char* TITLE = "Lua Console";

	lua_State* m_State;

	char                  InputBuf[256];
	ImVector<char*>       Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool                  ScrollToBottom;
};