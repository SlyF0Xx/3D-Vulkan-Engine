#pragma once

#include "imgui.h"
#include "widgets/GroupPanel.h"

#include <Engine.h>
#include <Core/Base.h>

#include "SceneInteraction.h"
#include "Constants.h"

namespace Editor {

// Inspector component replace.
#ifndef INS_COM_REP
#define INS_COM_REP m_Context->get_registry().emplace_or_replace
#endif

	class BaseComponentInspector {
	public:
		BaseComponentInspector() = delete;
		explicit BaseComponentInspector(EDITOR_GAME_TYPE ctx);

		void SetContext(EDITOR_GAME_TYPE game);

		void Render();

	protected:
		template<class T>
		inline T* GetComponent(entt::entity entity) const;

		bool IsAvailable() const;
		virtual bool IsRenderable() const;

		virtual void RenderContent() = 0;

		inline virtual const char* GetTitle() const = 0;

	protected:
		EDITOR_GAME_TYPE m_Context;
		entt::entity m_Selection;
	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT		= false;
	};

	template<class T>
	inline T* BaseComponentInspector::GetComponent(entt::entity entity) const {
		return m_Context->get_registry().try_get<T>(entity);
	}

}
