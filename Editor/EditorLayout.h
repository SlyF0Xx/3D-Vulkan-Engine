#pragma once

#include <Engine.h>

#include <vector>

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "ImGUIBasedPresentationEngine.h"
//#include "Engine.h"

namespace Editor {

	//using namespace diffusion;

	class EditorLayout {
	public:
		EditorLayout() {};
		virtual void Render(Game& vulkan, ImGUIBasedPresentationEngine& engine) = 0;
		void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine);
		ImVec2 GetSceneSize() const;
		
	protected:
		ImVec2 m_SceneSize;
		std::vector<ImTextureID> m_TexIDs;

	};

}