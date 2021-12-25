#pragma once

#include "ImGUIBasedPresentationEngine.h"

#include <Engine.h>
#include <Core/Base.h>

#include "BaseWidget.h"
#include "Constants.h"

namespace Editor {

	/// <summary>
	/// Контекстно-зависимый виджет.
	/// </summary>
	class GameWidget : public Widget {
	public:
		GameWidget() = delete;
		explicit GameWidget(EDITOR_GAME_TYPE ctx);

		virtual void SetContext(EDITOR_GAME_TYPE game);
		virtual void OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine);

		static ImTextureID GenerateTextureID(EDITOR_GAME_TYPE ctx, diffusion::ImageData& imData, const std::filesystem::path& path);
	protected:
		// diffusion::Ref<Game> m_Context;
		EDITOR_GAME_TYPE m_Context;
	};

}