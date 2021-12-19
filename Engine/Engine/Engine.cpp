// Engine.cpp : Defines the exported functions for the DLL.
//

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>

#include "Archiver.h"
#include "Engine.h"
#include "DeferredRender.h"
#include "ForwardRender.h"
#include "BaseComponents/BoundingComponent.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/PossessedComponent.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/MeshComponent.h"
#include "BaseComponents/DirectionalLightComponent.h"
#include "BaseComponents/PointLightComponent.h"
#include "BaseComponents/TagComponent.h"
#include "BaseComponents/DebugComponent.h"

#include "util.h"

#include "edyn/edyn.hpp"
#include <edyn/time/time.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Game::~Game()
{
}

vk::PhysicalDevice Game::select_physical_device()
{
    auto devices = m_instance.enumeratePhysicalDevices();

    vk::PhysicalDevice selected_device = devices[0];
    // Select GPU
    {
        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        for (auto& device : devices) {
            auto device_properties = device.getProperties();
            if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                selected_device = device;
            }
        }
    }

    return selected_device;
}

void Game::select_graphics_queue_family()
{
    auto queue_families_properties = m_phys_device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_families_properties.size(); i++) {
        if (queue_families_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            m_queue_family_index = i;
            break;
        }
    }
}

// This is the constructor of a class that has been exported.
Game::Game()
    : m_initializer(*this),
    m_component_initializer(*this)
{
    m_registry.clear();
    vk::ApplicationInfo application_info("Lab1", 1, "Engine", 1, VK_API_VERSION_1_2);

    std::vector<vk::LayerProperties> enlayers = vk::enumerateInstanceLayerProperties();

    std::ofstream fout("layers.txt");
    for (auto& layer : enlayers) {
        fout << layer.layerName << std::endl;
    }

    std::vector<vk::ExtensionProperties> ext = vk::enumerateInstanceExtensionProperties();
    /*
    std::array layers = {
        //"VK_LAYER_Galaxy_Overlay_DEBUG",
        //"VK_LAYER_Galaxy_Overlay_VERBOSE",
        //"VK_LAYER_Galaxy_Overlay"
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_LUNARG_api_dump",
        //  "VK_LAYER_LUNARG_monitor",

          //"VK_LAYER_RENDERDOC_Capture",
  #ifdef VK_TRACE
          "VK_LAYER_LUNARG_vktrace",
  #endif
    };
    */
    std::array<const char* const, 0> layers = {};

    std::array extensions = {
          //"VK_EXT_debug_utils",
        //"VK_KHR_external_memory_capabilities",
        //"VK_NV_external_memory_capabilities",
        "VK_EXT_swapchain_colorspace",
        "VK_KHR_surface",
        "VK_KHR_win32_surface"/*,
        "VK_KHR_get_physical_device_properties2"*/
    };

    //std::array<const char* const, 0> extensions = {};

    m_instance = vk::createInstance(vk::InstanceCreateInfo({}, &application_info, layers, extensions));
    m_phys_device = select_physical_device();

    vk::PhysicalDeviceFeatures required_features;
    required_features.imageCubeArray = true;
    required_features.geometryShader = true;

    auto features = m_phys_device.getFeatures();
    if (!features.imageCubeArray) {
        throw std::exception("Vulkan cube array is not supported by your device!");
    }

    if (!features.geometryShader) {
        throw std::exception("Vulkan geometry shaders is not supported by your device!");
    }

    select_graphics_queue_family();
    if (m_queue_family_index == -1) {
        throw std::exception("Vulkan queue family not found!");
    }

    std::array queue_priorities{ 1.0f };
    std::array queue_create_infos{ vk::DeviceQueueCreateInfo{{}, m_queue_family_index, queue_priorities} };
    std::array device_extensions{ "VK_KHR_swapchain" };
    m_device = m_phys_device.createDevice(vk::DeviceCreateInfo({}, queue_create_infos, {}, device_extensions, &required_features));

    m_queue = m_device.getQueue(m_queue_family_index, 0);
    m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo({}, 0));
    //m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0));

    vma::AllocatorCreateInfo allocator_info({}, m_phys_device, m_device);
    allocator_info.instance = m_instance;
    allocator_info.vulkanApiVersion = VK_API_VERSION_1_1;
    m_allocator = vma::createAllocator(allocator_info);

    m_lua_state = diffusion::create_lua_state(m_registry);



    edyn::init();
    edyn::attach(m_registry);
    edyn::set_fixed_dt(m_registry, 0.014);
}

void Game::InitializePresentationEngine(const PresentationEngine& presentation_engine)
{
    m_presentation_engine = presentation_engine;

    // TODO: move in presentation engine
    if (m_presentation_engine.m_color_format == vk::Format::eB8G8R8A8Unorm) {
        m_texture_component_mapping = vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR);
    }

    InitializePipelineLayout();

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    //render = new DeferredRender(*this, images);
    render = std::make_unique<ForwardRender>(*this, m_registry);
}

void Game::SecondInitialize()
{
    render->Update();
    for (int i = 0; i < m_presentation_engine.m_image_count; ++i) {
        m_presentation_engine.m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

        render->Initialize(i, m_presentation_engine.m_swapchain_data[i].m_command_buffer);
        if (m_menu_renderer) {
            m_menu_renderer->Render(m_presentation_engine.m_swapchain_data[i].m_command_buffer);
        }

        m_presentation_engine.m_swapchain_data[i].m_command_buffer.end();
    }

    m_initialized = true;
}

void Game::InitializePipelineLayout()
{
    std::array view_proj_binding{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr) /*view_proj*/
    };

    std::array world_binding{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
    };

    std::array material_bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr), /*albedo*/
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr) /*normal*/
    };

    std::array light_bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, nullptr) /*light*/
    };

    m_descriptor_set_layouts = {
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, view_proj_binding)),
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, world_binding)),
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, material_bindings)),
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, light_bindings)),
    };

    /*
    std::array push_constants = {
        vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, 4)
    };

    m_layout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_descriptor_set_layouts, push_constants));
    */
}

#if 0
void Game::Update(int width, int height)
{
    // Don't react to resize until after first initialization.
    if (!m_initialized) {
        return;
    }

    m_device.waitIdle();

    /*
    for (auto& swapchain_data : m_swapchain_data) {
        m_device.destroyFramebuffer(swapchain_data.m_deffered_framebuffer);
        m_device.destroyFramebuffer(swapchain_data.m_composite_framebuffer);


        m_device.destroyImageView(swapchain_data.m_color_image_view);
        m_device.destroyImageView(swapchain_data.m_depth_image_view);

        m_device.freeMemory(swapchain_data.m_depth_memory);

        m_device.destroyImage(swapchain_data.m_depth_image);
        // m_device.destroyImage(swapchain_data.m_color_image);
    }
    */
    m_width = width;
    m_height = height;

    std::array<uint32_t, 1> queues{ 0 };
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, m_image_count, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(width, height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate, VK_TRUE, m_swapchain));

    images = m_device.getSwapchainImagesKHR(m_swapchain);


    render->Update(images);
    m_device.freeCommandBuffers(m_command_pool, m_command_buffers);
    m_command_buffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_command_pool, vk::CommandBufferLevel::ePrimary, images.size()));
    for (int i = 0; i < images.size(); ++i) {
        m_swapchain_data[i].m_command_buffer = m_command_buffers[i];
        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

        render->Initialize(i, m_swapchain_data[i].m_command_buffer);
        if (m_menu_renderer) {
            m_menu_renderer->Render(m_swapchain_data[i].m_command_buffer);
        }

        m_swapchain_data[i].m_command_buffer.end();
    }
}
#endif

void Game::Update()
{
    // Don't react to resize until after first initialization.
    if (!m_initialized) {
        return;
    }

    m_device.waitIdle();

    render->Update();
    // WIN version only
    std::vector<vk::CommandBuffer> command_buffers;
    for (int i = 0; i < m_presentation_engine.m_image_count; ++i) {
        command_buffers.push_back(m_presentation_engine.m_swapchain_data[i].m_command_buffer);
    }
    m_device.freeCommandBuffers(m_command_pool, command_buffers);

    auto allocated_command_buffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_command_pool, vk::CommandBufferLevel::ePrimary, m_presentation_engine.m_image_count));
    for (int i = 0; i < m_presentation_engine.m_image_count; ++i) {
        m_presentation_engine.m_swapchain_data[i].m_command_buffer = allocated_command_buffers[i];
        m_presentation_engine.m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

        render->Initialize(i, m_presentation_engine.m_swapchain_data[i].m_command_buffer);
        if (m_menu_renderer) {
            m_menu_renderer->Render(m_presentation_engine.m_swapchain_data[i].m_command_buffer);
        }

        m_presentation_engine.m_swapchain_data[i].m_command_buffer.end();
    }
}

vk::ShaderModule Game::loadSPIRVShader(std::string filename)
{
    size_t shaderSize;
    char* shaderCode = nullptr;

    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open())
    {
        shaderSize = is.tellg();
        is.seekg(0, std::ios::beg);
        // Copy file contents into a buffer
        shaderCode = new char[shaderSize];
        is.read(shaderCode, shaderSize);
        is.close();
        assert(shaderSize > 0);
    }
    if (shaderCode)
    {
        auto shader = m_device.createShaderModule(vk::ShaderModuleCreateInfo({}, shaderSize, reinterpret_cast<const uint32_t*>(shaderCode)));

        delete[] shaderCode;

        return shader;
    }
    else
    {
        throw std::logic_error("Error: Could not open shader file \"" + filename + "\"");
    }
}

/*
void Game::update_light(int index, const LightInfo & light)
{
    m_lights[index] = light;

    m_lights_memory_to_transfer.clear();
    m_lights_memory_to_transfer.reserve(sizeof(uint32_t) + m_lights.size() * sizeof(LightInfo));

    uint32_t size = static_cast<uint32_t>(m_lights.size());
    m_lights_memory_to_transfer.insert(m_lights_memory_to_transfer.end(), reinterpret_cast<std::byte*>(&size), reinterpret_cast<std::byte*>(&size) + sizeof(size));
    m_lights_memory_to_transfer.insert(m_lights_memory_to_transfer.end(), reinterpret_cast<std::byte*>(m_lights.begin()._Ptr), reinterpret_cast<std::byte*>(m_lights.end()._Ptr));
    update_buffer(*this, m_lights_memory_to_transfer.size() * sizeof(std::byte), m_lights_memory_to_transfer.data(), m_lights_buffer, vk::BufferUsageFlagBits::eUniformBuffer, 0);
}
*/

void Game::InitializeColorFormats(const std::vector<vk::SurfaceFormatKHR>& formats, PresentationEngine& presentation_engine)
{
    /*
    std::vector<vk::Format> depth_formats{
        vk::Format::eB8G8R8A8Unorm,
        vk::Format::eD16Unorm,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD32Sfloat,
        vk::Format::eD32SfloatS8Uint };
    
    for (auto& format : depth_formats) {
        vk::FormatProperties format_features = m_phys_device.getFormatProperties(format);
        vk::FormatProperties2 format_props = m_phys_device.getFormatProperties2(format);
        bool optimal_supported = (format_features.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        bool linear_supported = (format_features.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        bool buffer_supported = (format_features.bufferFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        bool optimal_supported2 = (format_props.formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        bool linear_supported2 = (format_props.formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        bool buffer_supported2 = (format_props.formatProperties.bufferFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT) == vk::FormatFeatureFlagBits::eSampledImageFilterCubicEXT;
        if (optimal_supported || linear_supported || optimal_supported2 || linear_supported2 || buffer_supported || buffer_supported2) {
            std::cout << "kek";
        }
    }
    */
    for (auto& format : formats) {
        switch (format.format)
        {
        case vk::Format::eB8G8R8A8Unorm: {
            presentation_engine.m_color_format = format.format;
            m_texture_component_mapping = vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR);
            break;
        }
        case vk::Format::eR8G8B8A8Unorm: {
            presentation_engine.m_color_format = format.format;
            m_texture_component_mapping = vk::ComponentMapping();
            break;
        }


        case vk::Format::eD32SfloatS8Uint: {
            presentation_engine.m_depth_format = format.format;
            break;
        }
        case vk::Format::eD16UnormS8Uint: {
            presentation_engine.m_depth_format = format.format;
            break;
        }
        case vk::Format::eD24UnormS8Uint: {
            presentation_engine.m_depth_format = format.format;
            break;
        }
        }
    }

    if (presentation_engine.m_color_format == vk::Format::eUndefined) {
        throw std::exception("Color format is not supported");
    }

    if (presentation_engine.m_depth_format == vk::Format::eUndefined) {
        throw std::exception("Depth format is not supported");
    }
}

PresentationEngine Game::create_default_presentation_engine(HINSTANCE hinstance, HWND hwnd)
{
    PresentationEngine presentation_engine;

    presentation_engine.m_surface = m_instance.createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR({}, hinstance, hwnd));

    //safe check
    if (!m_phys_device.getSurfaceSupportKHR(0, presentation_engine.m_surface)) {
        throw std::exception("device doesn't support surface");
    }

    presentation_engine.m_depth_format = vk::Format::eD32SfloatS8Uint;
    InitializeColorFormats(m_phys_device.getSurfaceFormatsKHR(presentation_engine.m_surface), presentation_engine);

    auto capabilities = m_phys_device.getSurfaceCapabilitiesKHR(presentation_engine.m_surface);
    presentation_engine.m_width = capabilities.currentExtent.width;
    presentation_engine.m_height = capabilities.currentExtent.height;

    if (capabilities.maxImageCount > 2) {
        presentation_engine.m_image_count = 3;
    }

    if (!(capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque)) {
        throw std::exception("Supported Composite Alpha is not supported");
    }

    if (!(capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eColorAttachment)) {
        throw std::exception("Supported Usage Flags is not supported");
    }

    auto present_modes = m_phys_device.getSurfacePresentModesKHR(presentation_engine.m_surface);

    if (auto it = std::find(present_modes.begin(), present_modes.end(), vk::PresentModeKHR::eImmediate); it == present_modes.end()) {
        throw std::exception("Present mode is not supported");
    }
    presentation_engine.m_presentation_mode = vk::PresentModeKHR::eImmediate/*TODO*/;

    std::array<uint32_t, 1> queues{ 0 };
    presentation_engine.m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, presentation_engine.m_surface, presentation_engine.m_image_count, presentation_engine.m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(presentation_engine.m_width, presentation_engine.m_height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentation_engine.m_presentation_mode, VK_TRUE));

    auto images = m_device.getSwapchainImagesKHR(presentation_engine.m_swapchain);

    auto command_buffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_command_pool, vk::CommandBufferLevel::ePrimary, images.size()));
    presentation_engine.m_swapchain_data.resize(images.size());
    for (int i = 0; i < images.size(); ++i) {
        presentation_engine.m_swapchain_data[i].m_command_buffer = command_buffers[i];
        presentation_engine.m_swapchain_data[i].m_color_image = images[i];

        auto depth_allocation = m_allocator.createImage(
            vk::ImageCreateInfo({}, vk::ImageType::e2D, presentation_engine.m_depth_format, vk::Extent3D(presentation_engine.m_width, presentation_engine.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
            vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
        presentation_engine.m_swapchain_data[i].m_depth_image = depth_allocation.first;
        presentation_engine.m_swapchain_data[i].m_depth_memory = depth_allocation.second;

        presentation_engine.m_swapchain_data[i].m_color_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, presentation_engine.m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        presentation_engine.m_swapchain_data[i].m_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, presentation_engine.m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, presentation_engine.m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        presentation_engine.m_swapchain_data[i].m_fence = m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    presentation_engine.m_sema_data.resize(images.size());
    for (int i = 0; i < presentation_engine.m_sema_data.size(); ++i) {
        presentation_engine.m_sema_data[i].m_image_acquired_sema = m_device.createSemaphore(vk::SemaphoreCreateInfo());
        presentation_engine.m_sema_data[i].m_render_complete_sema = m_device.createSemaphore(vk::SemaphoreCreateInfo());
    }
    presentation_engine.m_final_layout = vk::ImageLayout::ePresentSrcKHR;

    return presentation_engine;
}

float Game::get_depth(size_t x, size_t y)
{
    auto command_buffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_command_pool, vk::CommandBufferLevel::ePrimary, 1));
    vk::CommandBuffer command_buffer = command_buffers[0];

    std::array<uint32_t, 1> queues{ 0 };
    size_t buffer_size = m_presentation_engine.m_width * m_presentation_engine.m_height * sizeof(float);
    auto buffer_memory =
        m_allocator.createBuffer(vk::BufferCreateInfo({}, buffer_size, vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues),
            vma::AllocationCreateInfo(vma::AllocationCreateInfo({}, vma::MemoryUsage::eCpuOnly)));

    std::array regions{ vk::BufferImageCopy({}, 0, 0, {vk::ImageAspectFlagBits::eDepth, 0, 0, 1}, {}, {static_cast<uint32_t>(m_presentation_engine.m_width), static_cast<uint32_t>(m_presentation_engine.m_height), 1}) };

    auto depth_image = m_presentation_engine.m_swapchain_data[m_presentation_engine.FrameIndex].m_depth_image;

    auto image_aspects = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    std::array start_barrier{ vk::ImageMemoryBarrier({}, vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, 0, 0, depth_image, vk::ImageSubresourceRange(image_aspects, 0, 1, 0, 1)) };
    std::array finish_barrier{ vk::ImageMemoryBarrier(vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, depth_image, vk::ImageSubresourceRange(image_aspects, 0, 1, 0, 1)) };

    command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, start_barrier);
    command_buffer.copyImageToBuffer(depth_image, vk::ImageLayout::eDepthAttachmentOptimal, buffer_memory.first, regions);
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, finish_barrier);
    command_buffer.end();

    auto fence = m_device.createFence(vk::FenceCreateInfo());

    std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
    m_queue.submit(queue_submits, fence);

    m_device.waitForFences({ fence }, true, -1);


    void* mapped_data = nullptr;
    m_allocator.mapMemory(buffer_memory.second, &mapped_data);

    size_t index = y * m_presentation_engine.m_width + x;
    float texel = reinterpret_cast<float*>(mapped_data)[index];
    return texel;
}

void Game::load_scene(const std::filesystem::path& path)
{
    std::ifstream fin(path);
    std::string str{ std::istreambuf_iterator<char>(fin),
                     std::istreambuf_iterator<char>() };

    NJSONInputArchive json_in(str);
    entt::basic_snapshot_loader loader(m_registry);
    loader.entities(json_in)
        .component<diffusion::TransformComponent, diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh,
        diffusion::PossessedEntity, diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent,
        diffusion::MainCameraTag, diffusion::PointLightComponent, diffusion::DirectionalLightComponent, diffusion::TagComponent,
        diffusion::ScriptComponent, diffusion::debug_tag /* should be ignored in runtime*/>(json_in);

    auto main_entity = m_registry.view<diffusion::PossessedEntity>().front();
    m_registry.set<diffusion::PossessedEntity>(main_entity);

    auto main_camera = m_registry.view<diffusion::MainCameraTag>().front();
    m_registry.set<diffusion::MainCameraTag>(main_camera);
}

void Game::save_scene(const std::filesystem::path& path)
{
    NJSONOutputArchive output;
    entt::snapshot{ m_registry }
        .entities(output)
        .component<diffusion::TransformComponent, diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh,
        diffusion::PossessedEntity, diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent,
        diffusion::MainCameraTag, diffusion::PointLightComponent, diffusion::DirectionalLightComponent, diffusion::TagComponent,
        diffusion::ScriptComponent, diffusion::debug_tag /* should be ignored in runtime*/>(output);
    output.Close();
    std::string json_output = output.AsString();

    // Scene is generated and exported in sample_scene.json
    std::ofstream fout(path);
    fout << json_output;
}

void Game::render_tick()
{
    std::lock_guard lock(m_render_mutex);
    if (std::chrono::steady_clock::now() - m_script_time_point > std::chrono::milliseconds(100)) {
        if (!m_paused) {
            m_registry.view<diffusion::ScriptComponent>().each([this](const diffusion::ScriptComponent& script) {
                m_component_initializer.add_to_execution(m_registry, entt::to_entity(m_registry, script));
            });
        }

        m_script_time_point = std::chrono::steady_clock::now();
    }

    std::optional<tf::Task> physics;
    if (std::chrono::steady_clock::now() - m_phys_time_point > std::chrono::milliseconds(1)) {
        if (!m_paused) {
            physics = m_taskflow.emplace([this]() {
                edyn::update(m_registry);
                });
            for (auto& task : m_tasks) {
                physics->succeed(task);
            }
        }

        m_phys_time_point = std::chrono::steady_clock::now();
    }

    auto draw = m_taskflow.emplace([this]() {
        //Draw();
        DrawRestruct();
    });
    if (physics) {
        draw.succeed(*physics);
    }

    m_executor.run(m_taskflow);
    m_executor.wait_for_all();
    m_tasks.clear();
    m_taskflow.clear();
}

void Game::run()
{
    m_paused = false;
    save_scene("scene.tmp");
}

void Game::pause()
{
    m_paused = true;
}

void Game::resume()
{
    m_paused = false;
}

void Game::stop()
{
    std::lock_guard lock(m_render_mutex);
    m_paused = true;
    while (m_registry.alive() != 0) {
        const entt::entity* begin = m_registry.data();

        for (int i = 0; i < m_registry.size(); ++i) {
            if (m_registry.valid(begin[i])) {
                m_registry.destroy(begin[i]);
                break;
            }
        }
    }
    m_registry.clear();
    m_registry.compact();

    m_registry.unset<diffusion::PossessedEntity>();
    m_registry.unset<diffusion::MainCameraTag>();

    auto lights = m_registry.ctx<diffusion::VulkanDirectionalLights>();
    m_device.freeDescriptorSets(m_descriptor_pool, { lights.m_lights_descriptor_set });
    m_registry.unset<diffusion::VulkanDirectionalLights>();
    //m_registry.unset<diffusion::RotateTag>();
    
    m_initialized = false;
    load_scene("scene.tmp");
    m_initialized = true;
}

void Game::Exit()
{
    // Destroy presentation engine
    std::vector<vk::CommandBuffer> command_buffers;
    for (int i = 0; i < m_presentation_engine.m_image_count; ++i) {
        command_buffers.push_back(m_presentation_engine.m_swapchain_data[i].m_command_buffer);
        m_device.destroyFence(m_presentation_engine.m_swapchain_data[i].m_fence);

        m_device.destroyImageView(m_presentation_engine.m_swapchain_data[i].m_depth_image_view);
        m_device.destroyImageView(m_presentation_engine.m_swapchain_data[i].m_color_image_view);

        m_allocator.destroyImage(m_presentation_engine.m_swapchain_data[i].m_depth_image, m_presentation_engine.m_swapchain_data[i].m_depth_memory);


        m_device.destroySemaphore(m_presentation_engine.m_sema_data[i].m_image_acquired_sema);
        m_device.destroySemaphore(m_presentation_engine.m_sema_data[i].m_render_complete_sema);
    }
    m_device.freeCommandBuffers(m_command_pool, command_buffers);


    m_device.destroySwapchainKHR(m_presentation_engine.m_swapchain);
    m_instance.destroySurfaceKHR(m_presentation_engine.m_surface);



    m_device.destroyCommandPool(m_command_pool);
    m_device.destroy();
    m_instance.destroy();
}

void Game::Draw()
{
    vk::Semaphore image_acquired_semaphore = m_presentation_engine.m_sema_data[m_presentation_engine.SemaphoreIndex].m_image_acquired_sema;
    vk::Semaphore render_complete_semaphore = m_presentation_engine.m_sema_data[m_presentation_engine.SemaphoreIndex].m_render_complete_sema;

    auto next_image = m_device.acquireNextImageKHR(m_presentation_engine.m_swapchain, UINT64_MAX, image_acquired_semaphore);

    m_device.waitForFences(m_presentation_engine.m_swapchain_data[next_image.value].m_fence, VK_TRUE, -1);
    m_device.resetFences(m_presentation_engine.m_swapchain_data[next_image.value].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(image_acquired_semaphore, stage_flags, command_buffers, render_complete_semaphore) };
    m_queue.submit(queue_submits, m_presentation_engine.m_swapchain_data[next_image.value].m_fence);

    std::array wait_sems = { render_complete_semaphore };

    std::array results{ vk::Result() };
    m_queue.presentKHR(vk::PresentInfoKHR(wait_sems, m_presentation_engine.m_swapchain, next_image.value, results));
    m_presentation_engine.SemaphoreIndex = (m_presentation_engine.SemaphoreIndex + 1) % m_presentation_engine.m_image_count; // Now we can use the next set of semaphores
}

void Game::DrawRestruct()
{
    vk::Semaphore image_acquired_semaphore = m_presentation_engine.m_sema_data[m_presentation_engine.SemaphoreIndex].m_image_acquired_sema;
    vk::Semaphore render_complete_semaphore = m_presentation_engine.m_sema_data[m_presentation_engine.SemaphoreIndex].m_render_complete_sema;

    auto next_image = m_device.acquireNextImageKHR(m_presentation_engine.m_swapchain, UINT64_MAX, image_acquired_semaphore);
    m_presentation_engine.FrameIndex = next_image.value;
    m_device.waitForFences(m_presentation_engine.m_swapchain_data[next_image.value].m_fence, VK_TRUE, -1);
    m_device.resetFences(m_presentation_engine.m_swapchain_data[next_image.value].m_fence);

    m_device.waitIdle();

    m_device.resetCommandPool(m_command_pool);

    m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    render->Initialize(m_presentation_engine.use_frame_index_to_render ? m_presentation_engine.FrameIndex : m_presentation_engine.SemaphoreIndex, m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer);
    if (m_menu_renderer) {
        m_menu_renderer->Render(m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer);
    }

    m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer.end();

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_presentation_engine.m_swapchain_data[next_image.value].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(image_acquired_semaphore, stage_flags, command_buffers, render_complete_semaphore) };
    m_queue.submit(queue_submits, m_presentation_engine.m_swapchain_data[next_image.value].m_fence);

    std::array wait_sems = { render_complete_semaphore };

    std::array results{ vk::Result() };
    m_queue.presentKHR(vk::PresentInfoKHR(wait_sems, m_presentation_engine.m_swapchain, next_image.value, results));
    m_presentation_engine.SemaphoreIndex = (m_presentation_engine.SemaphoreIndex + 1) % m_presentation_engine.m_image_count; // Now we can use the next set of semaphores
}
