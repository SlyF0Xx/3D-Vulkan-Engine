#pragma once

#include "ImGUIBasedPresentationEngine.h"

#include <Engine.h>
#include <Core/Base.h>

#include "BaseWidget.h"

namespace Editor {

	class GameWidget : public Widget {
	public:
		GameWidget() = delete;
		explicit GameWidget(const diffusion::Ref<Game>& ctx);

		void SetContext(const diffusion::Ref<Game>& game);
		virtual void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine);

		static ImTextureID GenerateTextureID(diffusion::Ref<Game>& ctx, diffusion::ImageData& imData, const std::filesystem::path& path);
	protected:
		diffusion::Ref<Game> m_Context;
	};

}