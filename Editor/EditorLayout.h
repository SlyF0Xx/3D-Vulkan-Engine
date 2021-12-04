#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include "ImGUIBasedPresentationEngine.h"

namespace Editor {

	class EditorWindow;

	enum class LayoutRenderStatus {
		SUCCESS, EXIT
	};

	class EditorLayout {
	public:
		EditorLayout() = delete;
		EditorLayout(const diffusion::Ref<Game>& game);
		virtual LayoutRenderStatus Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		virtual void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		// TODO: IT'S TEMP SOLUTION!!!
		virtual void ImportScene() = 0;

		void SetContext(const diffusion::Ref<Game>& game);

	protected:
		EditorWindow* GetParent();

		diffusion::Ref<Game> m_Context;

	private:
		EditorWindow* m_Parent;

		friend class EditorWindow;
	};

}