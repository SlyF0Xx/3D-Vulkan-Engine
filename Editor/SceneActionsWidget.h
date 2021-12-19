#pragma once

#include "GameWidget.h"

namespace Editor {

	class SceneActionsWidget : public GameWidget {
	public:
		static inline constexpr const char* TITLE = "Actions";

		SceneActionsWidget() = delete;
		SceneActionsWidget(EDITOR_GAME_TYPE ctx);

		void Render(bool* p_open, ImGuiWindowFlags flags) override;
		void InitContexed() override;
	private:
		static inline constexpr const char* PLAY_ICON_PATH = "./misc/icons/play.png";
		static inline constexpr const char* STOP_ICON_PATH = "./misc/icons/stop.png";
		static inline constexpr const char* PAUSE_ICON_PATH = "./misc/icons/pause.png";

		bool m_IsPaused;
		bool m_IsStopped;

		diffusion::ImageData m_StopTexData;
		ImTextureID m_StopTex;
		diffusion::ImageData m_TexPlayData;
		ImTextureID m_PlayTex;
		diffusion::ImageData m_TexPauseData;
		ImTextureID m_PauseTex;
	};

}
