#pragma once

#include <Engine.h>
#include <Core/Base.h>

#include <vector>

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "ImGUIBasedPresentationEngine.h"
//#include "Engine.h"

namespace Editor {

	class EditorWindow;

	//using namespace diffusion;

	enum class RENDER_STATUS {
		SUCCESS, EXIT
	};

	class EditorLayout {
	public:
		EditorLayout() = default;
		EditorLayout(diffusion::Ref<EditorWindow>& parent) : m_Parent(parent) {};
		virtual RENDER_STATUS Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine);
		ImVec2 GetSceneSize() const;
		
	protected:
		diffusion::Ref<EditorWindow> m_Parent;
		ImVec2 m_SceneSize;
		std::vector<ImTextureID> m_TexIDs;

	};

}