#include "TriangleComponent.h"
#include "Engine.h"

#if 0
TriangleComponent::TriangleComponent(
    GameImpl& game,
    vk::Format color_format,
    vk::Format depth_format,
    int width,
    int height,
    const vk::PhysicalDeviceMemoryProperties& memory_props)
    :
    m_game(game),
    m_color_format(color_format),
    m_depth_format(depth_format),
    m_width(width),
    m_height(height),
    m_memory_props(memory_props)
{}

void TriangleComponent::Initialize(const std::vector<vk::Image>& swapchain_images)
{
    m_command_buffers = m_game.m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.m_command_pool, vk::CommandBufferLevel::ePrimary, 2));

    std::array<uint32_t, 1> queues{ 0 };
    m_infos.resize(swapchain_images.size());

    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_color_format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
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

    m_render_pass = m_game.m_device.createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass_description, dependencies));

    m_layout = m_game.m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, {}, {}));

    m_vertex_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\Prim.vert.spv");
    m_fragment_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\Prim.frag.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main"),
                       vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_width, m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_width, m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);
    // TODO dynamic viewport and scissors

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_FALSE /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_cache = m_game.m_device.createPipelineCache(vk::PipelineCacheCreateInfo());
    auto pipeline_result = m_game.m_device.createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, {}, m_layout, m_render_pass));
    m_pipeline = pipeline_result.value;





    m_verticies = { PrimitiveVertex{0.25f, 0.75f, 0.5f},
                    PrimitiveVertex{0.5f,  0.25f, 0.5f},
                    PrimitiveVertex{0.75f, 0.75f, 0.5f}
    };

    std::size_t buffer_size = m_verticies.size() * sizeof(PrimitiveVertex);
    m_vertex_buffer = m_game.m_device.createBuffer(vk::BufferCreateInfo({}, buffer_size, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive, queues));
    auto memory_buffer_req = m_game.m_device.getBufferMemoryRequirements(m_vertex_buffer);

    uint32_t buffer_index = m_game.find_appropriate_memory_type(memory_buffer_req, m_memory_props, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_vertex_memory = m_game.m_device.allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

    void* mapped_data = nullptr/*= malloc(buffer_size)*/;
    m_game.m_device.mapMemory(m_vertex_memory, {}, buffer_size, {}, &mapped_data);
    std::memcpy(mapped_data, m_verticies.data(), buffer_size);

    m_game.m_device.bindBufferMemory(m_vertex_buffer, m_vertex_memory, {});
    m_game.m_device.flushMappedMemoryRanges(vk::MappedMemoryRange(m_vertex_memory, {}, buffer_size));



    /*
    vk::DescriptorPool m_descriptor_pool = game.m_device.createDescriptorPool();

    std::array bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eInputAttachment)
    };

    std::array set_layouts{
        game.m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings))
    };
    m_descriptor_sets = game.m_device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, set_layouts));
    */

    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 1.0f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    for (int mat_i = 0; mat_i < swapchain_images.size(); ++mat_i) {
        m_infos[mat_i].m_fence = m_game.m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_infos[mat_i].m_sema = m_game.m_device.createSemaphore(vk::SemaphoreCreateInfo());

        m_infos[mat_i].m_depth_image = m_game.m_device.createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_depth_format, vk::Extent3D(m_width, m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        auto memory_req = m_game.m_device.getImageMemoryRequirements(m_infos[mat_i].m_depth_image);

        uint32_t image_index = m_game.find_appropriate_memory_type(memory_req, m_memory_props, vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_infos[mat_i].m_depth_memory = m_game.m_device.allocateMemory(vk::MemoryAllocateInfo(memory_req.size, image_index));
        m_game.m_device.bindImageMemory(m_infos[mat_i].m_depth_image, m_infos[mat_i].m_depth_memory, {});

        m_infos[mat_i].m_color_image = swapchain_images[mat_i];
        m_infos[mat_i].m_color_image_view = m_game.m_device.createImageView(vk::ImageViewCreateInfo({}, m_infos[mat_i].m_color_image, vk::ImageViewType::e2D, m_color_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_infos[mat_i].m_depth_image_view = m_game.m_device.createImageView(vk::ImageViewCreateInfo({}, m_infos[mat_i].m_depth_image, vk::ImageViewType::e2D, m_depth_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        std::array image_views{ m_infos[mat_i].m_color_image_view, m_infos[mat_i].m_depth_image_view };
        m_infos[mat_i].m_framebuffer = m_game.m_device.createFramebuffer(vk::FramebufferCreateInfo({}, m_render_pass, image_views, m_width, m_height, 1));


        m_infos[mat_i].m_command_buffer = m_command_buffers[mat_i];

        m_infos[mat_i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

        m_infos[mat_i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
        m_infos[mat_i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_infos[mat_i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_width, m_height)), colors), vk::SubpassContents::eInline);

        // m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game_impl->m_materials[mat_i].m_layout, 0, m_game_impl->m_materials[mat_i].m_descriptor_sets, {});

        m_infos[mat_i].m_command_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
        m_infos[mat_i].m_command_buffer.draw(3, 1, 0, 0);

        m_infos[mat_i].m_command_buffer.endRenderPass();

        m_infos[mat_i].m_command_buffer.end();
    }
}

vk::Semaphore TriangleComponent::Draw(int swapchain_image_index, vk::Semaphore wait_sema)
{
    m_game.m_device.waitForFences(m_infos[swapchain_image_index].m_fence, VK_TRUE, -1);
    m_game.m_device.resetFences(m_infos[swapchain_image_index].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_infos[swapchain_image_index].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(wait_sema, stage_flags, command_buffers, m_infos[swapchain_image_index].m_sema) };
    m_game.m_queue.submit(queue_submits, m_infos[swapchain_image_index].m_fence);

    return m_infos[swapchain_image_index].m_sema;
}

void TriangleComponent::DestroyResources()
{
    for (auto& info : m_infos) {
        m_game.m_device.destroyFramebuffer(info.m_framebuffer);

        m_game.m_device.destroyImageView(info.m_color_image_view);
        m_game.m_device.destroyImageView(info.m_depth_image_view);

        m_game.m_device.destroyImage(info.m_depth_image);
        // m_game_impl->m_device.destroyImage(m_game_impl->m_color_image);

        m_game.m_device.destroyRenderPass(m_render_pass);
    }

    m_game.m_device.freeCommandBuffers(m_game.m_command_pool, m_command_buffers);
}
#endif