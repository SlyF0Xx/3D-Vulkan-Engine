#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

#include <Engine.h>
#include <Core/Base.h>
#include "Archiver.h"

#include "BoundingComponent.h"
#include "CameraComponent.h"
#include "MeshComponent.h"
#include "Relation.h"
#include "LitMaterial.h"
#include "UnlitMaterial.h"
#include "TransformComponent.h"
#include "PossessedComponent.h"
#include "DirectionalLightComponent.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <fstream>

#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <stb_image.h>

#include "MenuRender.h"
#include "EditorLayout.h"

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif // _DEBUG

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	(void) flags; (void) object; (void) location; (void) messageCode; (void) pUserData; (void) pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

using namespace diffusion;

namespace Editor {
	
	enum class FONT_TYPE {
		PRIMARY_TEXT,
		SUBHEADER_TEXT,
		LUA_EDITOR_PRIMARY
	};

	class EditorWindow {
	public:
		EditorWindow() = default;
		EditorWindow(Ref<EditorLayout>& layout, Ref<Game>& context);
		~EditorWindow();

		bool Create();

		void StartMainLoop();
		void SetLayout(Ref<EditorLayout>& layout);
		void SetContext(Ref<Game>& context);
		void SetupStyle();

		static void LoadFonts();

		static ImFont* GetFont(Editor::FONT_TYPE type);
	private: 
		bool GLFWInit();
		void SetupVulkan();
		void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, const vk::Instance& instance,
			const vk::Device& device,
			const vk::PhysicalDevice& phys_device,
			uint32_t queue_index);
		PresentationEngine GeneratePresentationEngine(Game& game, ImGui_ImplVulkanH_Window* wd, int w, int h);
		void SetupImGuiContext();
		void UploadFonts();

	private:
		static inline constexpr const char* FAVICON_PATH = "./misc/icons/toolbar_icon.png";

		static std::map<FONT_TYPE, ImFont*> s_Fonts;

		int m_Width = 0;
		int m_Height = 0;

		bool						m_IsInitialized = false;
		bool						m_SwapChainRebuild = false;
		int							m_MinImageCount = 3;

		VkAllocationCallbacks*		m_Allocator = NULL;
		VkPipelineCache				m_PipelineCache = VK_NULL_HANDLE;
		VkDescriptorPool			m_DescriptorPool = VK_NULL_HANDLE;
		ImGui_ImplVulkanH_Window	m_MainWindowData;

		ImVec4 m_BackgroundColor = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);

		GLFWwindow* m_Window;
		Ref<Game> m_Context;
		Ref<EditorLayout> m_Layout;
		Ref<PresentationEngine> m_PresentationEngine;
		std::vector<std::pair<vk::Image, vma::Allocation>*> m_LatestAllocations;
	};

	static void GLFWErrorCallback(int error, const char* description) {
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	}

	static void check_vk_result(VkResult err) {
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

}