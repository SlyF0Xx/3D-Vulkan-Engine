#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "GameProject.h"
#include "ImGUIBasedPresentationEngine.h"

namespace Editor {

	class EditorWindow;

	enum class LayoutRenderStatus {
		SUCCESS, EXIT
	};

	class EditorLayout {
	public:
		EditorLayout();
		virtual LayoutRenderStatus Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		virtual void OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		virtual void OnContextChanged() = 0;

		// void SetContext(const diffusion::Ref<Game>& game);

	protected:
		EditorWindow* GetParent();

		// TODO: Deprecated.
		// diffusion::Ref<Game> m_Context;
		Game* m_Context;

	private:
		EditorWindow* m_Parent;

		friend class EditorWindow;
	};

}