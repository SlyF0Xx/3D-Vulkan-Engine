#pragma once

#include <PhysicsUtils.h>

#include "BaseComponentInspector.h"

namespace Editor {

	class PhysicsComponentInspector : public BaseComponentInspector {
	public:
		PhysicsComponentInspector() = delete;
		explicit PhysicsComponentInspector(EDITOR_GAME_TYPE ctx);

		void RenderContent() override;
	private:
		inline const char* GetTitle() const override;

		void OnRemoveComponent() override;

		bool IsRenderable() const override;
	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT = false;

		SceneEventDispatcher m_SceneDispatcher;
	};

}