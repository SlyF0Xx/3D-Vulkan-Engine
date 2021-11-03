#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <typeinfo>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "TextEditor.h"

#include <Engine.h>
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

#pragma region Widgets
#include "ContentBrowser.h"
#include "BooleanStates.h"
#include "LuaConsole.h"
#include "SceneHierarchy.h"
#pragma endregion

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

static VkAllocationCallbacks* g_Allocator = NULL;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
//static int                      g_MinImageCount = 2;
static int                      g_MinImageCount = 3;
static bool                     g_SwapChainRebuild = false;

static void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	(void) flags; (void) object; (void) location; (void) messageCode; (void) pUserData; (void) pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

static void SetupVulkan(const vk::Device& device) {
	VkResult err;

	// Create Descriptor Pool
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
	check_vk_result(err);
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(
	ImGui_ImplVulkanH_Window* wd,
	VkSurfaceKHR surface,
	int width,
	int height,
	const vk::Instance& instance,
	const vk::Device& device,
	const vk::PhysicalDevice& phys_device,
	uint32_t queue_index) {
	wd->Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, queue_index, wd->Surface, &res);
	if (res != VK_TRUE) {
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(phys_device, wd->Surface, requestSurfaceImageFormat, (size_t) IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(phys_device, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(instance, phys_device, device, wd, queue_index, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan(const vk::Instance& instance, const vk::Device& device) {
	vkDestroyDescriptorPool(device, g_DescriptorPool, g_Allocator);
}

static void CleanupVulkanWindow(const vk::Instance& instance, const vk::Device& device) {
	ImGui_ImplVulkanH_DestroyWindow(instance, device, &g_MainWindowData, g_Allocator);
}

class MenuRenderer : public IMenuRenderer {
public:
	MenuRenderer(Game& vulkan, ImGui_ImplVulkanH_Window* wd)
		: m_vulkan(vulkan), m_wd(wd) {}

	void Render(const vk::CommandBuffer& command_buffer) {
		ImDrawData* draw_data = ImGui::GetDrawData();
		if (!draw_data) {
			//not initialized yet
			return;
		}
		ImGui_ImplVulkanH_Frame* fd = &m_wd->Frames[m_vulkan.get_presentation_engine().FrameIndex];
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = m_wd->RenderPass;
			info.framebuffer = fd->Framebuffer;
			info.renderArea.extent.width = m_wd->Width;
			info.renderArea.extent.height = m_wd->Height;
			info.clearValueCount = 1;
			info.pClearValues = &m_wd->ClearValue;
			vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

		// Submit command buffer
		vkCmdEndRenderPass(command_buffer);
	}

private:
	Game& m_vulkan;
	ImGui_ImplVulkanH_Window* m_wd;
};

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data, const vk::Device& device, const vk::Queue& queue) {
	VkResult err;

	VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	err = vkAcquireNextImageKHR(device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
		check_vk_result(err);

		err = vkResetFences(device, 1, &fd->Fence);
		check_vk_result(err);
	}
	{
		err = vkResetCommandPool(device, fd->CommandPool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		check_vk_result(err);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		err = vkEndCommandBuffer(fd->CommandBuffer);
		check_vk_result(err);
		err = vkQueueSubmit(queue, 1, &info, fd->Fence);
		check_vk_result(err);
	}
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd, const vk::Queue& queue) {
	if (g_SwapChainRebuild)
		return;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd->Swapchain;
	info.pImageIndices = &wd->FrameIndex;
	VkResult err = vkQueuePresentKHR(queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

static void glfwErrorCallback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void exit(GLFWwindow* window, const vk::Instance& instance, const vk::Device& device) {
	// Cleanup
	VkResult err = vkDeviceWaitIdle(device);
	check_vk_result(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	CleanupVulkanWindow(instance, device);
	CleanupVulkan(instance, device);

	glfwDestroyWindow(window);
	glfwTerminate();
}

PresentationEngine generate_presentation_engine_from_imgui(Game& game, ImGui_ImplVulkanH_Window* wd) {
	PresentationEngine presentation_engine;

	//presentation_engine.m_width = wd->Width;
	//presentation_engine.m_height = wd->Height;

	presentation_engine.m_width = 500;
	presentation_engine.m_height = 500;
	presentation_engine.m_surface = wd->Surface;
	presentation_engine.m_swapchain = wd->Swapchain;
	//presentation_engine.m_color_format = vk::Format(wd->SurfaceFormat.format);

	presentation_engine.m_color_format = vk::Format(wd->SurfaceFormat.format);
	presentation_engine.m_depth_format = vk::Format::eD32SfloatS8Uint;
	presentation_engine.m_presentation_mode = vk::PresentModeKHR(wd->PresentMode);
	presentation_engine.m_image_count = wd->ImageCount;

	std::array<uint32_t, 1> queues {0};

	auto command_buffers = game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, presentation_engine.m_image_count));
	presentation_engine.m_swapchain_data.resize(presentation_engine.m_image_count);
	for (int i = 0; i < presentation_engine.m_image_count; ++i) {
		//presentation_engine.m_swapchain_data[i].m_color_image = wd->Frames[i].Backbuffer;
		//presentation_engine.m_swapchain_data[i].m_color_image_view = wd->Frames[i].BackbufferView;
		auto color_allocation = game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, presentation_engine.m_color_format, vk::Extent3D(500, 500, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		presentation_engine.m_swapchain_data[i].m_color_image = color_allocation.first;
		presentation_engine.m_swapchain_data[i].m_color_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, presentation_engine.m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));


		//presentation_engine.m_swapchain_data[i].m_command_buffer = wd->Frames[i].CommandBuffer;
		presentation_engine.m_swapchain_data[i].m_command_buffer = command_buffers[i];
		presentation_engine.m_swapchain_data[i].m_fence = wd->Frames[i].Fence;

		auto depth_allocation = game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, presentation_engine.m_depth_format, vk::Extent3D(presentation_engine.m_width, presentation_engine.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		presentation_engine.m_swapchain_data[i].m_depth_image = depth_allocation.first;
		presentation_engine.m_swapchain_data[i].m_depth_memory = depth_allocation.second;
		presentation_engine.m_swapchain_data[i].m_depth_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, presentation_engine.m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
	}

	presentation_engine.m_sema_data.resize(presentation_engine.m_image_count);
	for (int i = 0; i < presentation_engine.m_image_count; ++i) {
		presentation_engine.m_sema_data[i].m_image_acquired_sema = wd->FrameSemaphores[i].ImageAcquiredSemaphore;
		presentation_engine.m_sema_data[i].m_render_complete_sema = wd->FrameSemaphores[i].RenderCompleteSemaphore;
	}

	return presentation_engine;
}

void import_scene(Game& vulkan) {
	std::ifstream fin("sample_scene.json");
	std::string str {std::istreambuf_iterator<char>(fin),
					 std::istreambuf_iterator<char>()};

	NJSONInputArchive json_in(str);
	entt::basic_snapshot_loader loader(vulkan.get_registry());
	loader.entities(json_in)
		.component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
				   diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
				   diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent>(json_in);

	auto main_entity = vulkan.get_registry().view<diffusion::PossessedEntity>().front();
	vulkan.get_registry().set<diffusion::PossessedEntity>(main_entity);
	vulkan.get_registry().set<diffusion::MainCameraTag>(main_entity);
}

int main() {
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit())
		return 1;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Awesome editor window", nullptr, NULL);
	GLFWimage images[1];
	images[0].pixels = stbi_load("./misc/icons/toolbar_icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);

	// Setup Vulkan
	if (!glfwVulkanSupported()) {
		printf("GLFW: Vulkan Not Supported\n");
		return 1;
	}
	uint32_t extensions_count = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

	diffusion::Ref<Game> vulkan = diffusion::CreateRef<Game>();
	SetupVulkan(vulkan->get_device());

	// Create Window Surface
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(vulkan->get_instance(), window, g_Allocator, &surface);
	check_vk_result(err);

	// Create Framebuffers
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

	SetupVulkanWindow(wd, surface, w, h, vulkan->get_instance(), vulkan->get_device(), vulkan->get_physical_device(), vulkan->get_queue_family_index());

	auto presentation_engine = generate_presentation_engine_from_imgui(*vulkan.get(), wd);
	vulkan->InitializePresentationEngine(presentation_engine);


	import_scene(*vulkan.get());


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
	ImGui::GetStyle().FrameRounding = 4.f;
	ImGui::GetStyle().FrameBorderSize = 1;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vulkan->get_instance();
	init_info.PhysicalDevice = vulkan->get_physical_device();
	init_info.Device = vulkan->get_device();
	init_info.QueueFamily = vulkan->get_queue_family_index();
	init_info.Queue = vulkan->get_queue();
	init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Allocator = g_Allocator;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

	// Load Fonts
	ImFont* font = io.Fonts->AddFontFromFileTTF("./misc/fonts/Roboto-Medium.ttf", 16.0f);
	ImFont* fontCodeEditor = io.Fonts->AddFontFromFileTTF("./misc/fonts/Droid-Sans-Mono.ttf", 24.0f);
	IM_ASSERT(font != NULL && fontCodeEditor != NULL);

	// Upload Fonts
	{
		// Use any command queue
		VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
		VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

		err = vkResetCommandPool(vulkan->get_device(), command_pool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);
		err = vkQueueSubmit(vulkan->get_queue(), 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(vulkan->get_device());
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	vulkan->register_menu_renderer(std::make_unique<MenuRenderer>(*vulkan.get(), wd));
	vulkan->SecondInitialize();

	std::vector<ImTextureID> tex_ids;
	for (auto& swapchain_data : presentation_engine.m_swapchain_data) {
		vk::Sampler color_sampler = vulkan->get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		tex_ids.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}

	// Our state
	ImVec4 clear_color = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);

	Editor::WindowStates s_windows = {};

	Editor::ContentBrowser browser = Editor::ContentBrowser();
	TextEditor textEditor = TextEditor();
	textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	textEditor.SetTabSize(4);
	textEditor.SetPalette(textEditor.GetLightPalette());
	textEditor.SetShowWhitespaces(false);

	LuaConsole luaConsole = LuaConsole();

	Editor::SceneHierarchy sceneHierarchy = Editor::SceneHierarchy(vulkan);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Resize swap chain?
		if (g_SwapChainRebuild) {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			if (width > 0 && height > 0) {
				ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(vulkan->get_instance(), vulkan->get_physical_device(), vulkan->get_device(), &g_MainWindowData, vulkan->get_queue_family_index(), g_Allocator, width, height, g_MinImageCount);
				g_MainWindowData.FrameIndex = 0;
				g_SwapChainRebuild = false;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		bool ds = true;
		ImGui::Begin("DockSpace", &ds, window_flags);

		// DockSpace
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		static auto first_time = true;
		if (first_time) {
			first_time = false;

			ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			// Order important!
			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
			ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
			ImGuiID dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.05f, nullptr, &dock_main_id);
			ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
			ImGuiID dock_down_right_id = ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

			ImGui::DockBuilderDockWindow("Actions", dock_up_id);
			ImGui::DockBuilderDockWindow(Editor::SceneHierarchy::TITLE, dock_left_id);
			ImGui::DockBuilderDockWindow("Right", dock_right_id);
			ImGui::DockBuilderDockWindow(Editor::ContentBrowser::TITLE, dock_down_id);
			ImGui::DockBuilderDockWindow("Project", dock_down_right_id);
			ImGui::DockBuilderDockWindow("Top2", dock_main_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}

		ImGui::End();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("New");
				ImGui::Separator();
				if (ImGui::MenuItem("Quit")) {
					exit(window, vulkan->get_instance(), vulkan->get_device());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows")) {
				if (ImGui::MenuItem("Content Browser")) {
					Editor::RevertBoolState(s_windows.isContentBrowserOpen);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		sceneHierarchy.Render();

		if (s_windows.isContentBrowserOpen) {
			browser.Render(&s_windows.isContentBrowserOpen, 0);
		}

		ImGui::Begin("Top2");
		ImGui::PushFont(fontCodeEditor);
		textEditor.Render("Editor", ImVec2(0, 0), true);
		ImGui::PopFont();
		ImGui::End();

		ImGui::Begin("Right");
		ImGui::Image(tex_ids[vulkan->get_presentation_engine().FrameIndex], ImVec2(500, 500));
		ImGui::Text("Hello, Right!");
		ImGui::End();

		luaConsole.Render();
		ImGui::ShowDemoWindow();

		// Rendering
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized) {
			wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			wd->ClearValue.color.float32[3] = clear_color.w;
			//FrameRender(wd, draw_data, vulkan->get_device(), vulkan->get_queue());
			//FramePresent(wd, vulkan->get_queue());

			//vulkan.get_device().resetCommandPool();
			//vkResetCommandPool(device, fd->CommandPool, 0);

			vulkan->DrawRestruct();
		}
	}

	exit(window, vulkan->get_instance(), vulkan->get_device());

	return 0;
}