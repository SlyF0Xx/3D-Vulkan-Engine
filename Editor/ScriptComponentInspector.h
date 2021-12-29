#pragma once

#include <BaseComponents/ScriptComponent.h>

#include "BaseComponentInspector.h"

namespace Editor {

	class ScriptComponentInspector : public BaseComponentInspector {
	public:
		ScriptComponentInspector() = delete;
		explicit ScriptComponentInspector(EDITOR_GAME_TYPE ctx);

		void RenderContent() override;
	private:
		inline const char* GetTitle() const override;

		void OnRemoveComponent() override;

		bool IsRenderable() const override;

		std::string GetSize() const;
	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT = false;

		std::string m_SizeStr;

		SceneEventDispatcher m_SceneDispatcher;

		diffusion::ScriptComponent* m_Component = nullptr;
	};

}