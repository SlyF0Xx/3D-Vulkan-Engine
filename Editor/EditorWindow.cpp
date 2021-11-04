#include "EditorWindow.h"

// LNK2001.
std::map<Editor::FONT_TYPE, ImFont*> Editor::EditorWindow::s_Fonts;

Editor::EditorWindow::EditorWindow(Ref<EditorLayout>& layout, Ref<Game>& context) {
	SetLayout(layout);
	SetContext(context);
}

Editor::EditorWindow::~EditorWindow() {
	// Cleanup
	VkResult err = vkDeviceWaitIdle(m_Context->get_device());
	check_vk_result(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(m_Context->get_device(), m_DescriptorPool, m_Allocator);
	ImGui_ImplVulkanH_DestroyWindow(
		m_Context->get_instance(),
		m_Context->get_device(),
		&m_MainWindowData,
		m_Allocator
	);

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

bool Editor::EditorWindow::Create() {
	if (!m_Context || !m_Layout) {
		return false;
	}

	if (!GLFWInit()) {
		return false;
	}
	SetupVulkan();

	// Create Window Surface
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(m_Context->get_instance(), m_Window, m_Allocator, &surface);
	check_vk_result(err);

	// Create Framebuffers
	glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
	ImGui_ImplVulkanH_Window* wd = &m_MainWindowData;
	SetupVulkanWindow(wd, surface, m_Context->get_instance(), m_Context->get_device(), m_Context->get_physical_device(), m_Context->get_queue_family_index());

	GeneratePresentationEngine(*m_Context, wd, 100, 100);
	m_Context->InitializePresentationEngine(*m_PresentationEngine);

	SetupImGuiContext();
	UploadFonts();

	m_Context->register_menu_renderer(std::make_unique<MenuRenderer>(*m_Context, &m_MainWindowData));
	m_IsInitialized = true;
	return true;
}

bool Editor::EditorWindow::GLFWInit() {
	glfwSetErrorCallback(Editor::GLFWErrorCallback);
	if (!glfwInit())
		return false;

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

	m_Window = glfwCreateWindow(1280, 720, "Awesome editor window", nullptr, NULL);
	GLFWimage images[1];
	images[0].pixels = stbi_load(FAVICON_PATH, &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(m_Window, 1, images);
	stbi_image_free(images[0].pixels);

	if (!glfwVulkanSupported()) {
		printf("GLFW: Vulkan Not Supported\n");
		return false;
	}
	return true;
}

void Editor::EditorWindow::SetupVulkan() {
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
	err = vkCreateDescriptorPool(m_Context->get_device(), &pool_info, m_Allocator, &m_DescriptorPool);
	check_vk_result(err);
}

void Editor::EditorWindow::SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, const vk::Instance& instance,
	const vk::Device& device,
	const vk::PhysicalDevice& phys_device,
	uint32_t queue_index) {
	wd->Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_Context->get_physical_device(), m_Context->get_queue_family_index(), wd->Surface, &res);
	if (res != VK_TRUE) {
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_Context->get_physical_device(), wd->Surface, requestSurfaceImageFormat, (size_t) IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_Context->get_physical_device(), wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(m_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(instance, phys_device, device, wd, queue_index, m_Allocator, m_Width, m_Height, m_MinImageCount);
}

PresentationEngine Editor::EditorWindow::GeneratePresentationEngine(Game& game, ImGui_ImplVulkanH_Window* wd, int w, int h) {
	int width = w <= 0 ? m_Width : w;
	int height = h <= 0 ? m_Height : h;

	if (m_PresentationEngine == nullptr) {
		m_PresentationEngine = CreateRef<PresentationEngine>();
	}

	//PresentationEngine presentation_engine;

	//m_PresentationEngine->m_width = wd->Width;
	//m_PresentationEngine->m_height = wd->Height;

	m_PresentationEngine->m_width = width;
	m_PresentationEngine->m_height = height;
	m_PresentationEngine->m_surface = wd->Surface;
	m_PresentationEngine->m_swapchain = wd->Swapchain;
	//m_PresentationEngine->m_color_format = vk::Format(wd->SurfaceFormat.format);

	m_PresentationEngine->m_color_format = vk::Format(wd->SurfaceFormat.format);
	m_PresentationEngine->m_depth_format = vk::Format::eD32SfloatS8Uint;
	m_PresentationEngine->m_presentation_mode = vk::PresentModeKHR(wd->PresentMode);
	m_PresentationEngine->m_image_count = wd->ImageCount;

	m_PresentationEngine->m_final_layout = vk::ImageLayout::eGeneral;


	std::array<uint32_t, 1> queues {0};

	auto command_buffers = game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, m_PresentationEngine->m_image_count));
	m_PresentationEngine->m_swapchain_data.resize(m_PresentationEngine->m_image_count);

	for (int i = 0; i < m_PresentationEngine->m_image_count; ++i) {
		//m_PresentationEngine->m_swapchain_data[i].m_color_image = wd->Frames[i].Backbuffer;
		//m_PresentationEngine->m_swapchain_data[i].m_color_image_view = wd->Frames[i].BackbufferView;
		auto color_allocation = game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, m_PresentationEngine->m_color_format, vk::Extent3D(width, height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		m_PresentationEngine->m_swapchain_data[i].m_color_image = color_allocation.first;
		m_PresentationEngine->m_swapchain_data[i].m_color_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_PresentationEngine->m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_PresentationEngine->m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));

		m_LatestAllocations.push_back(&color_allocation);


		//m_PresentationEngine->m_swapchain_data[i].m_command_buffer = wd->Frames[i].CommandBuffer;
		m_PresentationEngine->m_swapchain_data[i].m_command_buffer = command_buffers[i];
		m_PresentationEngine->m_swapchain_data[i].m_fence = wd->Frames[i].Fence;

		auto depth_allocation = game.get_allocator().createImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, m_PresentationEngine->m_depth_format, vk::Extent3D(m_PresentationEngine->m_width, m_PresentationEngine->m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
			vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
		m_PresentationEngine->m_swapchain_data[i].m_depth_image = depth_allocation.first;
		m_PresentationEngine->m_swapchain_data[i].m_depth_memory = depth_allocation.second;
		m_PresentationEngine->m_swapchain_data[i].m_depth_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_PresentationEngine->m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_PresentationEngine->m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

		m_LatestAllocations.push_back(&depth_allocation);
	}

	m_PresentationEngine->m_sema_data.resize(m_PresentationEngine->m_image_count);
	for (int i = 0; i < m_PresentationEngine->m_image_count; ++i) {
		m_PresentationEngine->m_sema_data[i].m_image_acquired_sema = wd->FrameSemaphores[i].ImageAcquiredSemaphore;
		m_PresentationEngine->m_sema_data[i].m_render_complete_sema = wd->FrameSemaphores[i].RenderCompleteSemaphore;
	}

	return *m_PresentationEngine;
}

void Editor::EditorWindow::SetupImGuiContext() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	SetupStyle();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_Window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_Context->get_instance();
	init_info.PhysicalDevice = m_Context->get_physical_device();
	init_info.Device = m_Context->get_device();
	init_info.QueueFamily = m_Context->get_queue_family_index();
	init_info.Queue = m_Context->get_queue();
	init_info.PipelineCache = m_PipelineCache;
	init_info.DescriptorPool = m_DescriptorPool;
	init_info.Allocator = m_Allocator;
	init_info.MinImageCount = m_MinImageCount;
	init_info.ImageCount = m_MainWindowData.ImageCount;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, m_MainWindowData.RenderPass);
}

void Editor::EditorWindow::UploadFonts() {
	LoadFonts();

	// Use any command queue
	VkCommandPool command_pool = m_MainWindowData.Frames[m_MainWindowData.FrameIndex].CommandPool;
	VkCommandBuffer command_buffer = m_MainWindowData.Frames[m_MainWindowData.FrameIndex].CommandBuffer;

	VkResult err = vkResetCommandPool(m_Context->get_device(), command_pool, 0);
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
	err = vkQueueSubmit(m_Context->get_queue(), 1, &end_info, VK_NULL_HANDLE);
	check_vk_result(err);

	err = vkDeviceWaitIdle(m_Context->get_device());
	check_vk_result(err);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Editor::EditorWindow::StartMainLoop() {
	if (!m_IsInitialized) return;
	// Main loop
	while (!glfwWindowShouldClose(m_Window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Resize swap chain?
		if (m_SwapChainRebuild) {
			glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
			if (m_Width > 0 && m_Height > 0) {
				ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(
					m_Context->get_instance(),
					m_Context->get_physical_device(),
					m_Context->get_device(),
					&m_MainWindowData,
					m_Context->get_queue_family_index(),
					m_Allocator,
					m_Width,
					m_Height,
					m_MinImageCount
				);
				m_MainWindowData.FrameIndex = 0;

				ImVec2 sceneSize = m_Layout->GetSceneSize();
				GeneratePresentationEngine(*m_Context, &m_MainWindowData, sceneSize.x, sceneSize.y);
				m_Context->InitializePresentationEngine(*m_PresentationEngine);
				m_Context->SecondInitialize();

				m_Layout->OnResize(*m_Context, *m_PresentationEngine);

				m_SwapChainRebuild = false;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImVec2 sceneSize = m_Layout->GetSceneSize();

		GeneratePresentationEngine(*m_Context, &m_MainWindowData, sceneSize.x, sceneSize.y);
		m_Context->InitializePresentationEngine(*m_PresentationEngine);
		m_Context->SecondInitialize();

		m_Layout->Render(*m_Context, *m_PresentationEngine);

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized) {
			m_MainWindowData.ClearValue.color.float32[0] = m_BackgroundColor.x * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[1] = m_BackgroundColor.y * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[2] = m_BackgroundColor.z * m_BackgroundColor.w;
			m_MainWindowData.ClearValue.color.float32[3] = m_BackgroundColor.w;

			try {
				m_Context->DrawRestruct();

				for (auto vmai : m_LatestAllocations) {
					m_Context->get_allocator().destroyImage(vmai->first, vmai->second);
				}
				m_LatestAllocations.clear();
			} catch (vk::OutOfDateKHRError& out_of_date) {
				m_SwapChainRebuild = true;
			}
		}
	}
}

void Editor::EditorWindow::SetLayout(Ref<EditorLayout>& layout) {
	m_Layout = layout;
}

void Editor::EditorWindow::SetContext(Ref<Game>& context) {
	m_Context = context;
}

void Editor::EditorWindow::SetupStyle() {
	ImGui::StyleColorsLight();
	ImGui::GetStyle().FrameRounding = 4.f;
	ImGui::GetStyle().FrameBorderSize = 1;
}

void Editor::EditorWindow::LoadFonts() {
	ImGuiIO& io = ImGui::GetIO();
	if (GetFont(FONT_TYPE::PRIMARY_TEXT) == NULL) {
		s_Fonts[FONT_TYPE::PRIMARY_TEXT] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Roboto-Medium.ttf", 16.0f);
		IM_ASSERT(GetFont(FONT_TYPE::PRIMARY_TEXT) != NULL);
	}

	if (GetFont(FONT_TYPE::SUBHEADER_TEXT) == NULL) {
		s_Fonts[FONT_TYPE::SUBHEADER_TEXT] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Roboto-Medium.ttf", 12.0f);
		IM_ASSERT(GetFont(FONT_TYPE::SUBHEADER_TEXT) != NULL);
	}

	if (GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY) == NULL) {
		s_Fonts[FONT_TYPE::LUA_EDITOR_PRIMARY] = io.Fonts->AddFontFromFileTTF("./misc/fonts/Droid-Sans-Mono.ttf", 24.0f);
		IM_ASSERT(GetFont(FONT_TYPE::LUA_EDITOR_PRIMARY) != NULL);
	}
}

ImFont* Editor::EditorWindow::GetFont(Editor::FONT_TYPE type) {
	if (s_Fonts.count(type) > 0) {
		return s_Fonts[type];
	}
	return nullptr;
}
