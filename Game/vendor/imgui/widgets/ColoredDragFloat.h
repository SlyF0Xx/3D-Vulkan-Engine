#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui {
	bool DragFloatN_Colored(const char* label, float* v, int components, float v_speed = 1.f, float v_min = 0.f, float v_max = 0.f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
}
