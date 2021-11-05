#pragma once

#include <Engine.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

namespace Editor {

class ImGUIBasedPresentationEngine
{
public:
	ImGUIBasedPresentationEngine(Game& game, ImGui_ImplVulkanH_Window* wd, int width, int height)
		: m_game(game), m_wd(wd)
	{
		generate_presentation_engine_from_imgui(width, height);
	}

	PresentationEngine& get_presentation_engine()
	{
		return presentation_engine;
	}

	void resize(int width, int height);

private:
	void generate_presentation_engine_from_imgui(int width, int height);

	Game& m_game;
	ImGui_ImplVulkanH_Window* m_wd;
	PresentationEngine presentation_engine;
	std::vector<vma::Allocation> m_color_allocation;
};

} // namespace Editor {