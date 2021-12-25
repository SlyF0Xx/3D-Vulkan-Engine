#include "EditorWindow.h"

Editor::EditorWindow::EditorWindow() {
	m_WindowDispatcher = WindowTitleInteractionSingleTon::GetDispatcher();
	m_WindowDispatcher->appendListener(Editor::WindowTitleInteractionType::TITLE_UPDATED, [&](void) {
		glfwSetWindowTitle(m_Window, GetWindowTitle().c_str());
	});

	m_WindowDispatcher->appendListener(Editor::WindowTitleInteractionType::CONTEXT_CHANGED, [&](void) {
		OnContextChanged();
	});

	// TODO: Deprecated.
	m_Context = GameProject::Instance()->GetCurrentContext();
}

Editor::EditorWindow::~EditorWindow() {
	Destroy();
}

bool Editor::EditorWindow::Create() {
	if (!CreateGLFW() || !CreateImGui()) {
		return false;
	}

	return m_IsInitialized;
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

	m_Window = glfwCreateWindow(1280, 720, GetWindowTitle().c_str(), nullptr, NULL);
	GLFWimage images[1];
	images[0].pixels = stbi_load(FAVICON_PATH, &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(m_Window, 1, images);
	stbi_image_free(images[0].pixels);

	// Necessary to use class's member function.
	glfwSetWindowUserPointer(m_Window, this);

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* w, int x, int y) {
		static_cast<EditorWindow*>(glfwGetWindowUserPointer(w))->GLFWResizeCallback(w, x, y);
	});

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

void Editor::EditorWindow::SetupImGuiContext() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGuiContext* context = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = NULL;

	if (io.BackendPlatformUserData != NULL) {
		return;
	}

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

	ImGuizmo::SetImGuiContext(context);
}

void Editor::EditorWindow::UploadFonts() {
	FontUtils::LoadFonts();

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

void Editor::EditorWindow::OnContextChanged() {
#if _DEBUG
	printf("------- Context changed (EditorWindow) -------");
#endif
	DestroyImGui();

	m_Context = GameProject::Instance()->GetCurrentContext();

	if (!CreateImGui()) {
		throw;
	}
	m_Layout->OnContextChanged();
	/*auto ptr = m_Layout->Copy();
	SetLayout(ptr);*/

	GameProject::Instance()->Refresh();

	glfwSetWindowTitle(m_Window, GetWindowTitle().c_str());

	StartMainLoop();
}

bool Editor::EditorWindow::CreateGLFW() {
	if (!GLFWInit()) {
		return false;
	}
	return true;
}

bool Editor::EditorWindow::CreateImGui() {
	if (!m_Context) {
		return false;
	}

	SetupVulkan();

	// Create Window Surface
	VkResult err = glfwCreateWindowSurface(m_Context->get_instance(), m_Window, m_Allocator, &m_Surface);
	check_vk_result(err);

	// Create Framebuffers
	glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
	m_LastWidth = m_Width;
	m_LastHeight = m_Height;
	ImGui_ImplVulkanH_Window* wd = &m_MainWindowData;
	SetupVulkanWindow(wd, m_Surface, m_Context->get_instance(), m_Context->get_device(), m_Context->get_physical_device(), m_Context->get_queue_family_index());

	m_PresentationEngine = diffusion::CreateRef<ImGUIBasedPresentationEngine>(*m_Context, wd, 100, 100);
	m_Context->InitializePresentationEngine(m_PresentationEngine->get_presentation_engine());

	SetupImGuiContext();
	UploadFonts();

	m_Context->register_menu_renderer(std::make_unique<MenuRenderer>(*m_Context, &m_MainWindowData));
	m_IsInitialized = true;
	return m_IsInitialized;
}

void Editor::EditorWindow::DestroyGLFW() {
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Editor::EditorWindow::DestroyImGui() {
	FontUtils::ReleaseFonts();

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
}

void Editor::EditorWindow::GLFWResizeCallback(GLFWwindow* window, int x, int y) {
	if (x == 0 || y == 0) {
		printf("Incorrect resizing size of window. Abort.");
		return;
	}
	printf("Resizing window to %d;x%d;", x, y);

	// Best looking on FHD.
	float relativeScale = static_cast<float>(x) / 1920.f;
	for (const auto& viewport : ImGui::GetCurrentContext()->Viewports) {
		ImGui::ScaleWindowsInViewport(viewport, relativeScale);
	}
	ImGui::GetStyle().ScaleAllSizes(relativeScale);
	ImGui::GetIO().FontGlobalScale = relativeScale;

	m_LastWidth = m_Width;
	m_LastHeight = m_Height;
	m_Width = x;
	m_Height = y;

	SetupStyle(relativeScale);

	m_Layout->OnResize(m_Context, *m_PresentationEngine);
}

void Editor::EditorWindow::Destroy() {
	if (!m_IsInitialized) return;

	DestroyImGui();
	DestroyGLFW();

	m_IsInitialized = false;
}

void Editor::EditorWindow::SetLayout(Ref<EditorLayout>& layout) {
	if (m_Layout != nullptr) {
		m_Layout->m_Parent = nullptr;
	}

	m_Layout = layout;
	m_Layout->m_Parent = this;
}

void Editor::EditorWindow::SetContext(EDITOR_GAME_TYPE context) {
	m_Context = context;
}

std::string Editor::EditorWindow::GetWindowTitle() const {
	return "Awesome Editor Window";
}

void Editor::EditorWindow::SetupStyle(float scale) {
	ImGui::StyleColorsLight();
	ImGui::GetStyle().WindowMinSize			= {100.f * scale, 100.f * scale};
	ImGui::GetStyle().ItemSpacing			= {4.f * scale, 4.f * scale};
	ImGui::GetStyle().ItemInnerSpacing		= {4.f * scale, 4.f * scale};
	ImGui::GetStyle().FramePadding			= {4.f * scale, 4.f * scale};
	ImGui::GetStyle().ScrollbarSize			= 8.f * scale;
	ImGui::GetStyle().FrameRounding			= 4.f * scale;
	ImGui::GetStyle().GrabMinSize			= 10.f * scale;
	ImGui::GetStyle().WindowPadding			= {0.f, 0.f};
	ImGui::GetStyle().WindowBorderSize		= 0.f;
	ImGui::GetStyle().FrameBorderSize		= 0.f;
	ImGui::GetStyle().ChildBorderSize		= 0.f;
}
