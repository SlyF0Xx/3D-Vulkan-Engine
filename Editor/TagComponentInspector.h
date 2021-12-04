#pragma once

#include "BaseComponents/TagComponent.h"

#include "BaseComponentInspector.h"
#include "Constants.h"

namespace Editor {

	class TagComponentInspector : public BaseComponentInspector {
	public:
		TagComponentInspector() = delete;
		explicit TagComponentInspector(const diffusion::Ref<Game>& ctx);

		void RenderContent() override;

	private:
		void Rename();

		inline const char* GetTitle() const override;

		bool IsRenderable() const override;

	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT	= false;

		SceneEventDispatcher m_SceneDispatcher;

		char m_RenameBuf[Constants::ACTOR_NAME_LENGTH]				= "";
		diffusion::TagComponent* m_TagComponent						= nullptr;
		bool m_IsFocused											= false;
	};

	

}
