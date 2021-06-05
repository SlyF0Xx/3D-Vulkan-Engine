// Engine.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Engine.h"

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

void Game::Initialize(HINSTANCE hinstance, HWND hwnd, int width, int height)
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

    m_sema = m_device.createSemaphore(vk::SemaphoreCreateInfo());

    std::array<uint32_t, 1> queues{ 0 };
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, 2, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(width, height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate/*TODO*/, VK_TRUE));

    auto images = m_device.getSwapchainImagesKHR(m_swapchain);
    m_swapchain_data.resize(images.size());

    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_color_image = images[i];
    }

    {
        std::array bindings{
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
        };

        m_descriptor_set_layouts = {
            m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings))
        };
        m_layout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_descriptor_set_layouts, {}));
    }

    {
        std::array bindings{
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr) /*albedo*/
        };

        m_composite_descriptor_set_layouts = {
            m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings)),
            m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings))
        };

        m_composite_layout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_composite_descriptor_set_layouts, {}));
    }
}


void Game::SecondInitialize()
{
    std::array<uint32_t, 1> queues{ 0 };
    /*
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_color_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
                                        vk::AttachmentDescription{{}, m_depth_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };
    */
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_color_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal},
                                        vk::AttachmentDescription{{}, m_depth_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };


    vk::AttachmentReference color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depth_attachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::array subpass_description{ vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics,
                                                           {},
                                                           color_attachment,
                                                           {},
                                                           &depth_attachment) };

    /*
    std::array dependencies{ vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::DependencyFlagBits::eByRegion),
                             vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryRead, vk::DependencyFlagBits::eByRegion),
                            };
    */


    std::array dependencies{ vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                                   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                                                   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                                                   vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                                   vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite),
                             vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                   {},
                                                   vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
    };

    m_deffered_render_pass = m_device.createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass_description, dependencies));





























    std::array game_component_attachment_descriptions{ vk::AttachmentDescription{{}, m_color_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
                                                       vk::AttachmentDescription{{}, m_depth_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };

    m_composite_render_pass = m_device.createRenderPass(vk::RenderPassCreateInfo({}, game_component_attachment_descriptions, subpass_description, dependencies));









    auto command_buffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_command_pool, vk::CommandBufferLevel::ePrimary, 2));
    /*
    for (int i = 0; i < images.size(); ++i) {
        m_swapchain_data[i].m_command_buffer = command_buffers[i];
    }
    InitializeDefferedPipeline();
    */





    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 2) };
    auto descriptor_pool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 2, pool_size));

    auto descriptor_set = m_device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_composite_descriptor_set_layouts));

    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    std::vector<vk::WriteDescriptorSet> write_descriptors;
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_descriptor_set = descriptor_set[i];

        m_swapchain_data[i].m_fence = m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_swapchain_data[i].m_sema = m_device.createSemaphore(vk::SemaphoreCreateInfo());

        m_swapchain_data[i].m_deffered_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_deffered_depth_image, m_swapchain_data[i].m_deffered_depth_memory);

        m_swapchain_data[i].m_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

        m_swapchain_data[i].m_albedo_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_color_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_albedo_image, m_swapchain_data[i].m_albedo_memory);

        //m_swapchain_data[i].m_color_image = images[i];
        m_swapchain_data[i].m_albedo_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_albedo_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_color_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
        m_swapchain_data[i].m_deffered_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_deffered_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        m_swapchain_data[i].m_albedo_sampler = m_device.createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
        std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_swapchain_data[i].m_albedo_sampler, m_swapchain_data[i].m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(descriptor_set[i], 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}));

        std::array deffered_views{ m_swapchain_data[i].m_albedo_image_view, m_swapchain_data[i].m_deffered_depth_image_view };
        m_swapchain_data[i].m_deffered_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_deffered_render_pass, deffered_views, m_width, m_height, 1));

        std::array image_views{ m_swapchain_data[i].m_color_image_view, m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_composite_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_composite_render_pass, image_views, m_width, m_height, 1));

        m_swapchain_data[i].m_command_buffer = command_buffers[i];
        /*
        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());
        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_swapchain_data[i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.endRenderPass();
        m_swapchain_data[i].m_command_buffer.end();
        */
    }
    m_device.updateDescriptorSets(write_descriptors, {});
    InitializeDefferedPipeline();



    m_initialized = true;
}

void Game::create_memory_for_image(const vk::Image & image, vk::DeviceMemory & memory)
{
    auto memory_req = m_device.getImageMemoryRequirements(image);

    uint32_t image_index = find_appropriate_memory_type(memory_req, m_memory_props, vk::MemoryPropertyFlagBits::eDeviceLocal);

    memory = m_device.allocateMemory(vk::MemoryAllocateInfo(memory_req.size, image_index));
    m_device.bindImageMemory(image, memory, {});
}

void Game::InitializeDefferedPipeline()
{
    {
        //E:\\programming\\Graphics\\Game\\Engine\\Engine\\

        m_vertex_shader = loadSPIRVShader("Deffered.vert.spv");
        m_fragment_shader = loadSPIRVShader("Deffered.frag.spv");

        std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main"),
                           vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_fragment_shader, "main")
        };

        std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
        std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof(float)) };

        vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
        vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

        std::array viewports{ vk::Viewport(0, 0, m_width, m_height, 0.0f, 1.0f) };
        std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_width, m_height)) };
        vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

        vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
        vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

        vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

        std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
        blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
        vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

        m_cache = get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

        std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

        auto pipeline_result = get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_deffered_render_pass));
        m_deffered_pipeline = pipeline_result.value;
    }









    {
        m_composite_vertex_shader = loadSPIRVShader("Composite.vert.spv");
        m_composite_fragment_shader = loadSPIRVShader("Composite.frag.spv");

        std::array stages = { vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_composite_vertex_shader, "main"),
                   vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_composite_fragment_shader, "main")
        };

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;
        vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

        std::array viewports{ vk::Viewport(0, 0, m_width, m_height, 0.0f, 1.0f) };
        std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_width, m_height)) };
        vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

        vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
        vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

        vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

        std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
        blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
        vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

        m_composite_cache = get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

        std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

        auto pipeline_result = get_device().createGraphicsPipeline(m_composite_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_composite_layout, m_composite_render_pass));
        m_composite_pipeline = pipeline_result.value;
    }



































    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                   vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };











    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());
        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_deffered_render_pass, m_swapchain_data[i].m_deffered_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_deffered_pipeline);

        vk::Viewport viewport(0, 0, m_width, m_height, 0.0f, 1.0f);
        m_swapchain_data[i].m_command_buffer.setViewport(0, viewport);
        vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_width, m_height));
        m_swapchain_data[i].m_command_buffer.setScissor(0, scissor);

        for (auto& [mat_type, materials] : materials_by_type) {
            for (auto& material : materials) {
                m_materials[material]->UpdateMaterial();
                for (auto& mesh : mesh_by_material[material]) {
                    mesh->Draw(m_swapchain_data[i].m_command_buffer);
                }
            }
        }

        m_swapchain_data[i].m_command_buffer.endRenderPass();

        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_composite_render_pass, m_swapchain_data[i].m_composite_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_composite_pipeline);

        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 0, m_swapchain_data[i].m_descriptor_set, {});
        m_swapchain_data[i].m_command_buffer.draw(3, 1, 0, 0);
        m_swapchain_data[i].m_command_buffer.endRenderPass();
        m_swapchain_data[i].m_command_buffer.end();
    }

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

    for (auto& swapchain_data : m_swapchain_data) {
        m_device.destroyFramebuffer(swapchain_data.m_deffered_framebuffer);
        m_device.destroyFramebuffer(swapchain_data.m_composite_framebuffer);


        m_device.destroyImageView(swapchain_data.m_color_image_view);
        m_device.destroyImageView(swapchain_data.m_depth_image_view);

        m_device.freeMemory(swapchain_data.m_depth_memory);

        m_device.destroyImage(swapchain_data.m_depth_image);
        // m_device.destroyImage(swapchain_data.m_color_image);
    }

    m_width = width;
    m_height = height;

    std::array<uint32_t, 1> queues{ 0 };
    m_swapchain = m_device.createSwapchainKHR(vk::SwapchainCreateInfoKHR({}, m_surface, 2, m_color_format, vk::ColorSpaceKHR::eSrgbNonlinear, vk::Extent2D(width, height), 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate, VK_TRUE, m_swapchain));

    auto images = m_device.getSwapchainImagesKHR(m_swapchain);
    m_swapchain_data.resize(images.size());

    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };


    std::vector<vk::WriteDescriptorSet> write_descriptors;
    for (int i = 0; i < images.size(); ++i) {
        m_swapchain_data[i].m_deffered_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_deffered_depth_image, m_swapchain_data[i].m_deffered_depth_memory);

        m_swapchain_data[i].m_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

        m_swapchain_data[i].m_albedo_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_color_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_albedo_image, m_swapchain_data[i].m_albedo_memory);

        m_swapchain_data[i].m_color_image = images[i];
        m_swapchain_data[i].m_albedo_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_albedo_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_color_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
        m_swapchain_data[i].m_deffered_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_deffered_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        m_swapchain_data[i].m_albedo_sampler = m_device.createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
        std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_swapchain_data[i].m_albedo_sampler, m_swapchain_data[i].m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}));

        std::array deffered_views{ m_swapchain_data[i].m_albedo_image_view, m_swapchain_data[i].m_deffered_depth_image_view };
        m_swapchain_data[i].m_deffered_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_deffered_render_pass, deffered_views, m_width, m_height, 1));

        std::array image_views{ m_swapchain_data[i].m_color_image_view, m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_composite_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_composite_render_pass, image_views, m_width, m_height, 1));
    }






#if 0
        m_swapchain_data[i].m_deffered_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_deffered_depth_image, m_swapchain_data[i].m_deffered_depth_memory);

        m_swapchain_data[i].m_depth_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

        m_swapchain_data[i].m_albedo_image = m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_color_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        create_memory_for_image(m_swapchain_data[i].m_albedo_image, m_swapchain_data[i].m_albedo_memory);

        m_swapchain_data[i].m_color_image = images[i];
        m_swapchain_data[i].m_albedo_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_albedo_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_color_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
        m_swapchain_data[i].m_deffered_depth_image_view = m_device.createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_deffered_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        std::array deffered_views{ m_swapchain_data[i].m_albedo_image_view, m_swapchain_data[i].m_deffered_depth_image_view };
        m_swapchain_data[i].m_deffered_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_deffered_render_pass, deffered_views, m_width, m_height, 1));

        std::array image_views{ m_swapchain_data[i].m_color_image_view, m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_composite_framebuffer = m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_composite_render_pass, image_views, m_width, m_height, 1));
#endif






    m_device.updateDescriptorSets(write_descriptors, {});

    for (int i = 0; i < images.size(); ++i) {
        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());
        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_deffered_render_pass, m_swapchain_data[i].m_deffered_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_deffered_pipeline);

        vk::Viewport viewport(0, 0, m_width, m_height, 0.0f, 1.0f);
        m_swapchain_data[i].m_command_buffer.setViewport(0, viewport);
        vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_width, m_height));
        m_swapchain_data[i].m_command_buffer.setScissor(0, scissor);

        for (auto& [mat_type, materials] : materials_by_type) {
            for (auto& material : materials) {
                m_materials[material]->UpdateMaterial();
                for (auto& mesh : mesh_by_material[material]) {
                    mesh->Draw(m_swapchain_data[i].m_command_buffer);
                }
            }
        }

        m_swapchain_data[i].m_command_buffer.endRenderPass();

        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_composite_render_pass, m_swapchain_data[i].m_composite_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_composite_pipeline);

        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 0, m_swapchain_data[i].m_descriptor_set, {});
        m_swapchain_data[i].m_command_buffer.draw(3, 1, 0, 0);
        m_swapchain_data[i].m_command_buffer.endRenderPass();
        m_swapchain_data[i].m_command_buffer.end();



    }

    for (auto& game_components : m_game_components) {
        game_components->Update(m_swapchain_data.size(), m_width, m_height);
    }
}

void Game::AddGameComponent(IGameComponent * component)
{
    m_game_components.push_back(component);
    m_game_components.back()->Initialize(m_swapchain_data.size());
}
/*
void Game::AddGameComponent(std::unique_ptr<IGameComponent> component)
{
    m_game_components.push_back(std::move(component));
    m_game_components.back()->Initialize(m_images);
}
*/
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

void Game::Exit()
{
    for (auto& game_component : m_game_components) {
        game_component->DestroyResources();
    }

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

    m_device.destroySwapchainKHR(m_swapchain);

    m_instance.destroySurfaceKHR(m_surface);
    m_device.destroyCommandPool(m_command_pool);
    m_device.destroy();
    m_instance.destroy();
}

void Game::Draw()
{
    auto next_image = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_sema);

    m_device.waitForFences(m_swapchain_data[next_image.value].m_fence, VK_TRUE, -1);
    m_device.resetFences(m_swapchain_data[next_image.value].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_swapchain_data[next_image.value].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(m_sema, stage_flags, command_buffers, m_swapchain_data[next_image.value].m_sema) };
    m_queue.submit(queue_submits, m_swapchain_data[next_image.value].m_fence);

#if 0
    std::vector<vk::Semaphore> semaphores;
    for (auto& game_component : m_game_components) {
        semaphores.push_back(game_component->Draw(next_image.value, m_swapchain_data[next_image.value].m_sema /*{ m_sema, m_swapchain_data[next_image.value].m_sema }*/));
    }
#endif
    std::array wait_sems = { m_swapchain_data[next_image.value].m_sema };

    std::array results{vk::Result()};
    try {
        m_queue.presentKHR(vk::PresentInfoKHR(wait_sems, m_swapchain, next_image.value, results));
    }
    catch (.../*const vk::IncompatibleDisplayKHRError & exception*/) {
        auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
        auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
        Update(screenWidth, screenHeight);
    }
}
