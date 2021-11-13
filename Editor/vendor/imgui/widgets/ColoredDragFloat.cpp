#include "ColoredDragFloat.h"

bool ImGui::DragFloatN_Colored(const char* label, float* v, int components, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	bool value_changed = false;
	ImGui::BeginGroup();
	ImGui::PushID(label);
	ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());
	for (int i = 0; i < components; i++) {
		static const ImU32 colors[] = {
			0xAA0000FF, // red
			0xAA00FF00, // green
			0xAAFF0000, // blue
			0xAAFFFFFF, // white for alpha?
		};

		ImGui::PushID(i);
		value_changed |= ImGui::DragFloat("##v", &v[i], v_speed, v_min, v_max, format, flags);

		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const float spacing = g.Style.FrameRounding;
		//const float spacing = 2;
		const float halfSpacing = spacing / 2;

		// This is the main change
		window->DrawList->AddLine({min.x + spacing, max.y - halfSpacing}, {max.x - spacing, max.y - halfSpacing}, colors[i], 2);

		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
		ImGui::PopID();
		ImGui::PopItemWidth();
	}
	ImGui::PopID();

	//ImGui::TextUnformatted(label, ImGui::FindRenderedTextEnd(label));
	ImGui::EndGroup();

	return value_changed;
}