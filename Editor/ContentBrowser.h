#pragma once

#include <filesystem>

#include "BaseWidget.h"

namespace Editor {

	class ContentBrowser : Widget {

	public:
		static inline constexpr const char* TITLE = "Content Browser";

		ContentBrowser();

		virtual void Render();
		virtual void Render(bool* p_open, ImGuiWindowFlags flags);

	private:
		std::filesystem::path m_CurrentDirectory;

	};

}