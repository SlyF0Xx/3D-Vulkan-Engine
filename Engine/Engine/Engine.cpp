// Engine.cpp : Defines the exported functions for the DLL.
//

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>

#include "Engine.h"
#include "DeferredRender.h"
#include "ForwardRender.h"

#include "util.h"

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

void Game::InitializeColorFormats(const std::vector<vk::SurfaceFormatKHR> & formats)
{
    for (auto& format : formats) {
        switch (format.format)
        {
        case vk::Format::eB8G8R8A8Unorm: {
            m_color_format = format.format;
            m_texture_component_mapping = vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR);
            break;
        }
        case vk::Format::eR8G8B8A8Unorm: {
            m_color_format = format.format;
            m_texture_component_mapping = vk::ComponentMapping();
            break;
        }


        case vk::Format::eD32SfloatS8Uint: {
            m_depth_format = format.format;
            break;
        }
        case vk::Format::eD16UnormS8Uint: {
            m_depth_format = format.format;
            break;
        }
        case vk::Format::eD24UnormS8Uint: {
            m_depth_format = format.format;
            break;
        }
        }
    }

    if (m_color_format == vk::Format::eUndefined) {
        throw std::exception("Color format is not supported");
    }

    if (m_depth_format == vk::Format::eUndefined) {
        throw std::exception("Depth format is not supported");
    }
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
    vk::ApplicationInfo application_info("Lab1", 1, "Engine", 1, VK_API_VERSION_1_2);

    std::vector<vk::LayerProperties> enlayers = vk::enumerateInstanceLayerProperties();

    std::ofstream fout("layers.txt");
    for (auto& layer : enlayers) {
        fout << layer.layerName << std::endl;
    }

    std::vector<vk::ExtensionProperties> ext = vk::enumerateInstanceExtensionProperties();
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
    //std::array<const char* const, 0> layers = {};

    std::array extensions = {
        "VK_EXT_debug_utils",
        //"VK_KHR_external_memory_capabilities",
        //"VK_NV_external_memory_capabilities",
        "VK_EXT_swapchain_colorspace",
        "VK_KHR_surface",
        "VK_KHR_win32_surface",

        //"VK_KHR_get_physical_device_properties2"
    };

    //std::array<const char* const, 0> extensions = {};

    m_instance = vk::createInstance(vk::InstanceCreateInfo({}, &application_info, layers, extensions));
    m_phys_device = select_physical_device();

    select_graphics_queue_family();
    if (m_queue_family_index == -1) {
        throw std::exception("Vulkan queue family not found!");
    }

    std::array queue_priorities{ 1.0f };
    std::array queue_create_infos{ vk::DeviceQueueCreateInfo{{}, m_queue_family_index, queue_priorities} };
    std::array device_extensions{ "VK_KHR_swapchain" };
    m_device = m_phys_device.createDevice(vk::DeviceCreateInfo({}, queue_create_infos, {}, device_extensions));

    m_queue = m_device.getQueue(m_queue_family_index, 0);
    m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo({}, 0));
}

void Game::InitializeSurface(vk::SurfaceKHR surface)
{
    m_surface = surface;

    //safe check
    if (!m_phys_device.getSurfaceSupportKHR(0, m_surface)) {
        throw std::exception("device doesn't support surface");
    }

    m_depth_format = vk::Format::eD32SfloatS8Uint;
    InitializeColorFormats(m_phys_device.getSurfaceFormatsKHR(m_surface));
    m_memory_props = m_phys_device.getMemoryProperties();

    auto capabilities = m_phys_device.getSurfaceCapabilitiesKHR(m_surface);
    m_width = capabilities.currentExtent.width;
    m_height = capabilities.currentExtent.height;

    if (capabilities.maxImageCount > 2) {
        m_image_count = 3;
    }

    if (!(capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque)) {
        throw std::exception("Supported Composite Alpha is not supported");
    }

    if (!(capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eColorAttachment)) {
        throw std::exception("Supported Usage Flags is not supported");
    }

    auto present_modes = m_phys_device.getSurfacePresentModesKHR(m_surface);

    if (auto it = std::find(present_modes.begin(), present_modes.end(), vk::PresentModeKHR::eImmediate); it == present_modes.end()) {
        throw std::exception("Present mode is not supported");
    }

    std::array<uint32_t, 1> queues{ 0 };
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, m_image_count, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(m_width, m_height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate/*TODO*/, VK_TRUE));

    InitializePipelineLayout();

    vma::AllocatorCreateInfo allocator_info({}, m_phys_device, m_device);
    allocator_info.instance = m_instance;
    allocator_info.vulkanApiVersion = VK_API_VERSION_1_1;
    m_allocator = vma::createAllocator(allocator_info);


    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));



    auto sets = m_device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, m_descriptor_set_layouts[3]));
    m_lights_descriptor_set = sets[0];

    auto out3 = sync_create_empty_host_invisible_buffer(*this, 10 * sizeof(LightShaderInfo), vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_lights_buffer = out3.m_buffer;
    m_lights_memory = out3.m_allocation;

    std::array lights_buffer_infos{ vk::DescriptorBufferInfo(m_lights_buffer, {}, VK_WHOLE_SIZE) };

    auto out4 = sync_create_empty_host_invisible_buffer(*this, sizeof(uint32_t), vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_lights_count_buffer = out4.m_buffer;
    m_lights_count_memory = out4.m_allocation;

    std::array lights_count_buffer_infos{ vk::DescriptorBufferInfo(m_lights_count_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_lights_descriptors{ vk::WriteDescriptorSet(m_lights_descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, lights_buffer_infos, {}),
                                         vk::WriteDescriptorSet(m_lights_descriptor_set, 1, 0, vk::DescriptorType::eUniformBuffer, {}, lights_count_buffer_infos, {}) };
    m_device.updateDescriptorSets(write_lights_descriptors, {});

    images = m_device.getSwapchainImagesKHR(m_swapchain);

    glm::mat4 ProjectionMatrix = glm::perspective(
        static_cast<float>(glm::radians(60.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
        16.0f / 9.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
        0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
        100.0f                                 // Дальняя плоскость отсечения.
    );
    m_shadpwed_lights = std::make_unique<Lights>(*this, ProjectionMatrix, images);


    //render = new DeferredRender(*this, images);
    render = new ForwardRender(*this, images, m_registry);
}

void Game::SecondInitialize()
{
    render->Initialize();

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
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, nullptr), /*light*/
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, nullptr)  /*light count*/
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

int Game::add_light(const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector)
{
    m_shadpwed_lights->add_light(position, cameraTarget, upVector);
    render->Update(images);

    //m_lights.push_back(light);
    std::vector<LightShaderInfo> light = m_shadpwed_lights->get_light_shader_info();

    m_lights_memory_to_transfer.clear();
    m_lights_memory_to_transfer.reserve(light.size() * sizeof(LightShaderInfo));
    m_lights_memory_to_transfer.insert(m_lights_memory_to_transfer.end(), reinterpret_cast<std::byte*>(light.begin()._Ptr), reinterpret_cast<std::byte*>(light.end()._Ptr));
    update_buffer(*this, m_lights_memory_to_transfer.size(), m_lights_memory_to_transfer.data(), m_lights_buffer, vk::BufferUsageFlagBits::eUniformBuffer, 0);

    uint32_t size = static_cast<uint32_t>(light.size());
    m_lights_count_memory_to_transfer.clear();
    m_lights_count_memory_to_transfer.reserve(sizeof(size));
    m_lights_count_memory_to_transfer.insert(m_lights_count_memory_to_transfer.end(), reinterpret_cast<std::byte*>(&size), reinterpret_cast<std::byte*>(&size) + sizeof(size));
    update_buffer(*this, m_lights_count_memory_to_transfer.size(), m_lights_count_memory_to_transfer.data(), m_lights_count_buffer, vk::BufferUsageFlagBits::eUniformBuffer, 0);

    return light.size() - 1;
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
void Game::Exit()
{
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

    m_device.destroyRenderPass(m_deffered_render_pass);
    m_device.destroyRenderPass(m_composite_render_pass);
    */
    m_device.destroySwapchainKHR(m_swapchain);

    m_instance.destroySurfaceKHR(m_surface);
    m_device.destroyCommandPool(m_command_pool);
    m_device.destroy();
    m_instance.destroy();
}

void Game::Draw()
{
    render->Draw();
}
