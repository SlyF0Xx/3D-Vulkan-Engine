#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "BaseWidget.h"

namespace Editor {

	class GameWidget : public Widget {
	public:
		GameWidget() = delete;
		explicit GameWidget(const diffusion::Ref<Game>& ctx);

		void SetContext(const diffusion::Ref<Game>& game);
	protected:
		diffusion::Ref<Game> m_Context;
	};

}