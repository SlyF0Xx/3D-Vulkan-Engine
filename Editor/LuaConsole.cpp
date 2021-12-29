#include "LuaConsole.h"

#include "BaseComponents/TagComponent.h"
#include "BaseComponents/TransformComponent.h"
#include "BaseComponents/VulkanComponents/ImportableVulkanMeshComponents.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

void printMessage(const std::string& s) {
	std::cout << s << std::endl;
}

Editor::LuaConsole::LuaConsole(EDITOR_GAME_TYPE vulkan) : Editor::GameWidget(vulkan) {
	ClearLog();
	memset(InputBuf, 0, sizeof(InputBuf));
	HistoryPos = -1;

	// "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
	Commands.push_back("HELP");
	Commands.push_back("HISTORY");
	Commands.push_back("PRINT(...)");
	Commands.push_back("CLEAR");
	Commands.push_back("CLASSIFY");
	AutoScroll = true;
	ScrollToBottom = false;

	// Lua.
	m_State = luaL_newstate();
	/* getGlobalNamespace(m_State)
		 .beginClass<LuaConsole>("console")
			 .addConstructor <void (*) (void)>()
			 .addFunction("print", &LuaPrint)
		 .endClass();*/
	luaL_openlibs(m_State);
	lua_settop(m_State, 0);

	getGlobalNamespace(m_State)
		.beginClass<entt::entity>("entity")
		.endClass();

	LoadImguiBindings(m_State);
	/*
	.beginClass<diffusion::TransformComponent>("transform")
	.endClass()
	.addFunction("get_transform_component", [this](entt::entity entity) {
		return m_vulkan->get_registry().get<diffusion::TransformComponent>(entity);
	})
	.addFunction("global_translate", [this](diffusion::TransformComponent& transform, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entt::to_entity(m_vulkan->get_registry(), transform),
			transform.m_world_matrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
		});
	})
	.addFunction("local_translate", [this](diffusion::TransformComponent& transform, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entt::to_entity(m_vulkan->get_registry(), transform),
			[x, y, z](auto& transform) {
			transform.m_world_matrix = glm::translate(transform.m_world_matrix, glm::vec3(x, y, z));
		});
	})
	*/
	/*.addFunction("global_translate", [this](entt::entity& entity, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entity, [x, y, z](auto& transform) {
			transform.m_world_matrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
		});
	})
	.addFunction("local_translate", [this](entt::entity & entity, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entity, [x, y, z](auto& transform) {
			transform.m_world_matrix = glm::translate(transform.m_world_matrix, glm::vec3(x, y, z));
		});
	})
	.addFunction("local_rotate", [this](entt::entity& entity, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entity, [x, y, z](auto& transform) {
			glm::vec3 RotationX(1.0, 0, 0);
			transform.m_world_matrix = glm::rotate(transform.m_world_matrix, x, RotationX);

			glm::vec3 RotationY(0, 1.0, 0);
			transform.m_world_matrix = glm::rotate(transform.m_world_matrix, y, RotationY);

			glm::vec3 RotationZ(0, 0, 1.0);
			transform.m_world_matrix = glm::rotate(transform.m_world_matrix, z, RotationZ);
		});
	})
	.addFunction("local_scale", [this](entt::entity& entity, float x, float y, float z) {
		m_vulkan->get_registry().patch<diffusion::TransformComponent>(entity, [x, y, z](auto& transform) {
			glm::mat4 scale_matrix = glm::scale(glm::vec3(x, y, z));
			transform.m_world_matrix *= scale_matrix;
		});
	})
	.addFunction("spawn_entity", [this]() {
		return m_vulkan->get_registry().create();
	})
	.addFunction("change_name", [this](entt::entity& entity, const char* name) {
		m_vulkan->get_registry().emplace_or_replace<diffusion::TagComponent>(entity, name);
	})
	.addFunction("add_transform", [this](entt::entity& entity) {
		m_vulkan->get_registry().emplace<diffusion::TransformComponent>(entity, glm::mat4(1));
	})
	.addFunction("import_mesh", [this](entt::entity& entity, const char* path) {
		diffusion::import_mesh(std::filesystem::path(path), m_vulkan->get_registry(), entity);
	})
	.addFunction("get_entity_by_name", [this](const char* name) { return get_entity_by_name(name); });*/

	//lua_getglobal(m_State, "_G");
	//luaL_Reg luas[] = {
	//    {"print", LuaPrint}
	//};
	//// luaL_register(L, NULL, printlib); // for Lua versions < 5.2
	//luaL_setfuncs(m_State, luas, 0);  // for Lua versions 5.2 or greater
	//lua_pop(m_State, 1);
}

Editor::LuaConsole::~LuaConsole() {
	ClearLog();
	for (int i = 0; i < History.Size; i++)
		free(History[i]);
}

void Editor::LuaConsole::ClearLog() {
	for (int i = 0; i < Items.Size; i++)
		free(Items[i]);
	Items.clear();
}

void Editor::LuaConsole::Render() {
	Render(0, 0);
}

void Editor::LuaConsole::Render(bool* p_open, ImGuiWindowFlags flags) {
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	if (!ImGui::Begin(TITLE, p_open, flags)) {
		ImGui::End();
		return;
	}

	// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
	// So e.g. IsItemHovered() will return true when hovering the title bar.
	// Here we create a context menu only available from the title bar.
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Close Console"))
			*p_open = false;
		ImGui::EndPopup();
	}

	/*ImGui::TextWrapped(
		"This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
		"implementation may want to store entries along with extra data such as timestamp, emitter, etc.");*/
	ImGui::TextWrapped("Enter 'HELP' for help.");

	// TODO: display items starting from the bottom

	/*if (ImGui::SmallButton("Add Debug Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
	ImGui::SameLine();
	if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
	ImGui::SameLine();*/
	if (ImGui::SmallButton("Clear")) { ClearLog(); }
	ImGui::SameLine();
	bool copy_to_clipboard = ImGui::SmallButton("Copy");
	//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

	ImGui::Separator();

	// Options menu
	if (ImGui::BeginPopup("Options")) {
		ImGui::Checkbox("Auto-scroll", &AutoScroll);
		ImGui::EndPopup();
	}

	// Options, Filter
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");
	ImGui::SameLine();
	Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::Separator();

	// Reserve enough left-over height for 1 separator + 1 input text
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::Selectable("Clear")) ClearLog();
		ImGui::EndPopup();
	}

	// Display every line as a separate entry so we can change their color or add custom widgets.
	// If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
	// to only process visible items. The clipper will automatically measure the height of your first item and then
	// "seek" to display only items in the visible area.
	// To use the clipper we can replace your standard loop:
	//      for (int i = 0; i < Items.Size; i++)
	//   With:
	//      ImGuiListClipper clipper;
	//      clipper.Begin(Items.Size);
	//      while (clipper.Step())
	//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
	// - That your items are evenly spaced (same height)
	// - That you have cheap random access to your elements (you can access them given their index,
	//   without processing all the ones before)
	// You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
	// We would need random-access on the post-filtered list.
	// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
	// or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
	// and appending newly elements as they are inserted. This is left as a task to the user until we can manage
	// to improve this example code!
	// If your items are of variable height:
	// - Split them into same height items would be simpler and facilitate random-seeking into your list.
	// - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	if (copy_to_clipboard)
		ImGui::LogToClipboard();
	for (int i = 0; i < Items.Size; i++) {
		const char* item = Items[i];
		if (!Filter.PassFilter(item))
			continue;

		// Normally you would store more information in your item than just a string.
		// (e.g. make Items[] an array of structure, store color/type etc.)
		ImVec4 color;
		bool has_color = false;
		if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; } else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); has_color = true; }
		//if (has_color)
		//	ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextUnformatted(item);
	/*	if (has_color)
			ImGui::PopStyleColor();*/
	}
	if (copy_to_clipboard)
		ImGui::LogFinish();

	if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		ImGui::SetScrollHereY(1.0f);
	ScrollToBottom = false;

	//ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	// Command-line
	bool reclaim_focus = false;
	ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
	if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*) this)) {
		char* s = InputBuf;
		Strtrim(s);
		if (s[0])
			ExecCommand(s);
		strcpy_s(s, 1, "");
		reclaim_focus = true;
	}

	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	//ImGui::PopStyleVar();
	ImGui::End();
}

void Editor::LuaConsole::ExecCommand(const char* command_line) {
	AddLog("# %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back.
	// This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = History.Size - 1; i >= 0; i--)
		if (Stricmp(History[i], command_line) == 0) {
			free(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	History.push_back(Strdup(command_line));

	// Process command
	if (Stricmp(command_line, "CLEAR") == 0) {
		ClearLog();
	} else if (Stricmp(command_line, "HELP") == 0) {
		AddLog("Commands:");
		for (int i = 0; i < Commands.Size; i++)
			AddLog("- %s", Commands[i]);
	} else if (Stricmp(command_line, "HISTORY") == 0) {
		int first = History.Size - 10;
		for (int i = first > 0 ? first : 0; i < History.Size; i++)
			AddLog("%3d: %s\n", i, History[i]);
	} else if (strstr(command_line, "PRINT(")) {
		std::string cmd = command_line;
		std::string value = cmd.substr(6, cmd.length() - 7);
		std::cout << value << std::endl;

		LuaRef reference = getGlobal(m_State, value.c_str());
		if (!reference.isNil()) {
			AddLog(reference.tostring().c_str());
		} else {
			AddLog(value.c_str());
		}
	} else {
		const int ret = luaL_dostring(m_State, command_line);
		if (ret != LUA_OK) {
			const char* str = lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			AddLog(str);
		}

		static int ojfd = 0;
		if (ojfd == 0) {
			//luaL_dostring(m_State, "imgui.End()");
			ojfd++;
		}



		//AddLog("Unknown command: '%s'\n", command_line);
	}

	// On command input, we scroll to bottom even if AutoScroll==false
	ScrollToBottom = true;
}

int Editor::LuaConsole::TextEditCallback(ImGuiInputTextCallbackData* data) {
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag) {
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				// Example of TEXT COMPLETION

				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf) {
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				// Build a list of candidates
				ImVector<const char*> candidates;
				for (int i = 0; i < Commands.Size; i++)
					if (Strnicmp(Commands[i], word_start, (int) (word_end - word_start)) == 0)
						candidates.push_back(Commands[i]);

				if (candidates.Size == 0) {
					// No match
					AddLog("No match for \"%.*s\"!\n", (int) (word_end - word_start), word_start);
				} else if (candidates.Size == 1) {
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
					data->DeleteChars((int) (word_start - data->Buf), (int) (word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]);
					data->InsertChars(data->CursorPos, " ");
				} else {
					// Multiple matches. Complete as much as we can..
					// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
					int match_len = (int) (word_end - word_start);
					for (;;) {
						int c = 0;
						bool all_candidates_matches = true;
						for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0) {
						data->DeleteChars((int) (word_start - data->Buf), (int) (word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
					}

					// List matches
					AddLog("Possible matches:\n");
					for (int i = 0; i < candidates.Size; i++)
						AddLog("- %s\n", candidates[i]);
				}

				break;
			}
			case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = HistoryPos;
				if (data->EventKey == ImGuiKey_UpArrow) {
					if (HistoryPos == -1)
						HistoryPos = History.Size - 1;
					else if (HistoryPos > 0)
						HistoryPos--;
				} else if (data->EventKey == ImGuiKey_DownArrow) {
					if (HistoryPos != -1)
						if (++HistoryPos >= History.Size)
							HistoryPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != HistoryPos) {
					const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
		}
		return 0;
	}
}

int Editor::LuaConsole::LuaPrint(const std::string& s) {
	std::cout << s << std::endl;
	return 0;
}

entt::entity Editor::LuaConsole::get_entity_by_name(const char* name) {
	std::string_view name_view(name);
	auto view = m_Context->get_registry().view<diffusion::TagComponent>();
	for (auto& entity : view) {
		auto& tag_component = m_Context->get_registry().get<diffusion::TagComponent>(entity);
		if (tag_component.m_Tag == name_view) {
			return entity;
		}
	}
	return entt::entity();
}

void Editor::LuaConsole::AddLog(const char* fmt, ...) IM_FMTARGS(2) {
	// FIXME-OPT
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	Items.push_back(Strdup(buf));
}
