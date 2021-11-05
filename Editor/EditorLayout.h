#pragma once

#include <Engine.h>

#include "ImGUIBasedPresentationEngine.h"

namespace Editor {

	class EditorWindow;

	enum class RENDER_STATUS {
		SUCCESS, EXIT
	};

	class EditorLayout {
	public:
		EditorLayout() = default;
		virtual RENDER_STATUS Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		virtual void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;

	protected:
		EditorWindow* GetParent();

	private:
		EditorWindow* m_Parent;

		friend class EditorWindow;
	};

}