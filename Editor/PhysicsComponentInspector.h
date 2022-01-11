#pragma once

#include <PhysicsUtils.h>

#include "BaseComponentInspector.h"

namespace Editor {

	class PhysicsComponentInspector : public BaseComponentInspector {
	public:
		PhysicsComponentInspector() = delete;
		explicit PhysicsComponentInspector(EDITOR_GAME_TYPE ctx);

		void RenderContent() override;
		void OnRegisterUpdated() override;
	private:
		inline const char* GetTitle() const override;

		void OnRemoveComponent() override;

		bool IsRenderable() const override;

		bool HasMass() const;
	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT = false;

		float m_MassInKg = 0.01f;

		SceneEventDispatcher m_SceneDispatcher;
	};

}