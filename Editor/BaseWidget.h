#pragma once

#include <imgui.h>

namespace Editor {

	class Widget {
	public:
		static inline constexpr const char* TITLE = "BaseWidget";

		virtual void Render() = 0;
		virtual void Render(bool* p_open, ImGuiWindowFlags flags) = 0;
	};

}