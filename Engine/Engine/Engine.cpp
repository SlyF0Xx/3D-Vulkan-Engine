// Engine.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Engine.h"
#include "DeferredRender.h"
#include "ForwardRender.h"

#include "util.h"

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

// This is the constructor of a class that has been exported.
Game::Game()
{
}

Game::~Game()
{
}

void Game::Initialize(HINSTANCE hinstance, HWND hwnd, int width, int height, const glm::mat4& CameraMatrix, const glm::mat4& ProjectionMatrix)
{
    m_width = width;
    m_height = height;

    vk::ApplicationInfo application_info("Lab1", 1, "Engine", 1, VK_API_VERSION_1_2);

    std::array layers = {
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_LUNARG_monitor",

        // "VK_LAYER_RENDERDOC_Capture",
#ifdef VK_TRACE
        "VK_LAYER_LUNARG_vktrace",
#endif
    };

    std::array extensions = {
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
        "VK_KHR_external_memory_capabilities",
        "VK_NV_external_memory_capabilities",
        "VK_EXT_swapchain_colorspace",
        "VK_KHR_surface",
        "VK_KHR_win32_surface",

        "VK_KHR_get_physical_device_properties2"
    };

    m_instance = vk::createInstance(vk::InstanceCreateInfo({}, &application_info, layers, extensions));

    auto devices = m_instance.enumeratePhysicalDevices();

    std::array queue_priorities{ 1.0f };
    std::array queue_create_infos{ vk::DeviceQueueCreateInfo{{}, 0, queue_priorities} };
    std::array device_extensions{ "VK_KHR_swapchain" };
    m_device = devices[0].createDevice(vk::DeviceCreateInfo({}, queue_create_infos, {}, device_extensions));
    m_queue = m_device.getQueue(0, 0);
    m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0));
    m_surface = m_instance.createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR({}, hinstance, hwnd));

    //safe check
    if (!devices[0].getSurfaceSupportKHR(0, m_surface)) {
        throw std::exception("device doesn't support surface");
    }

    auto formats = devices[0].getSurfaceFormatsKHR(m_surface);
    m_memory_props = devices[0].getMemoryProperties();

    std::array<uint32_t, 1> queues{ 0 };
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, 2, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(width, height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate/*TODO*/, VK_TRUE));

    InitializePipelineLayout();

    auto images = m_device.getSwapchainImagesKHR(m_swapchain);
    render = new DeferredRender(*this, images);
    //render = new ForwardRender(*this, images);






    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = m_device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, m_descriptor_set_layouts[0]))[0];

    m_view_projection_matrix = ProjectionMatrix * CameraMatrix;

    std::vector matrixes{ m_view_projection_matrix };
    auto out2 = create_buffer(*this, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
    m_world_view_projection_matrix_buffer = out2.m_buffer;
    m_world_view_projection_matrix_memory = out2.m_memory;
    m_world_view_projection_mapped_memory = out2.m_mapped_memory;

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_view_projection_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_device.updateDescriptorSets(write_descriptors, {});
}

void Game::SecondInitialize()
{
    render->Initialize();

    m_initialized = true;
}

void Game::InitializePipelineLayout()
{
    std::array view_proj_binding{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
    };

    std::array world_binding{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
    };

    m_descriptor_set_layouts = {
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, view_proj_binding)),
        m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, world_binding)),
    };
    m_layout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_descriptor_set_layouts, {}));
}

void Game::create_memory_for_image(const vk::Image & image, vk::DeviceMemory & memory)
{
    auto memory_req = m_device.getImageMemoryRequirements(image);

    uint32_t image_index = find_appropriate_memory_type(memory_req, m_memory_props, vk::MemoryPropertyFlagBits::eDeviceLocal);

    memory = m_device.allocateMemory(vk::MemoryAllocateInfo(memory_req.size, image_index));
    m_device.bindImageMemory(image, memory, {});
}

void Game::register_material(MaterialType material_type, /*std::unique_ptr<*/ IMaterial * /*>*/ material)
{
    m_materials.emplace(material->get_id(), material);
    //m_materials.emplace(std::pair(material->get_id(), std::move(material)));
    materials_by_type[material_type].insert(material->get_id());
}

void Game::register_mesh(int material_id, /*std::unique_ptr<*/IMesh * /*>*/ mesh)
{
    mesh_by_material[material_id].emplace(mesh);
    //mesh_by_material[material_id].emplace(std::move(mesh));
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
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, 2, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(width, height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate, VK_TRUE, m_swapchain));

    auto images = m_device.getSwapchainImagesKHR(m_swapchain);
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

uint32_t Game::find_appropriate_memory_type(vk::MemoryRequirements & mem_req, const vk::PhysicalDeviceMemoryProperties& memory_props, vk::MemoryPropertyFlags memory_flags)
{
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
        if ((mem_req.memoryTypeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memory_props.memoryTypes[i].propertyFlags & memory_flags) == memory_flags) {
                return i;
            }
        }
        mem_req.memoryTypeBits >>= 1;
    }
    return -1;
}

void Game::update_camera_projection_matrixes(const glm::mat4& CameraMatrix, const glm::mat4& ProjectionMatrix)
{
    m_view_projection_matrix = ProjectionMatrix * CameraMatrix;

    std::vector matrixes{ m_view_projection_matrix };

    auto memory_buffer_req = m_device.getBufferMemoryRequirements(m_world_view_projection_matrix_buffer);

    std::memcpy(m_world_view_projection_mapped_memory, matrixes.data(), sizeof(glm::mat4));
    m_device.flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_view_projection_matrix_memory, {}, memory_buffer_req.size));
}

void Game::update_world_matrix(const glm::mat4& world_matrix)
{
    /*
    std::vector matrixes{ world_matrix };

    auto memory_buffer_req = m_device.getBufferMemoryRequirements(m_world_view_projection_matrix_buffer);

    std::memcpy(static_cast<std::byte*>(m_world_view_projection_mapped_memory) + sizeof(glm::mat4), matrixes.data(), sizeof(glm::mat4));
    m_device.flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_view_projection_matrix_memory, {}, memory_buffer_req.size));
    */
}

void Game::Exit()
{

    m_device.unmapMemory(m_world_view_projection_matrix_memory);
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
