#include "ForwardRender.h"
#include "Engine.h"

ForwardRender::ForwardRender(Game& game, const std::vector<vk::Image>& swapchain_images)
    : m_game(game)
{
    m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());

    InitializeRenderPass();

    m_swapchain_data.resize(swapchain_images.size());
    InitializeConstantPerImage();
    InitializeVariablePerImage(swapchain_images);

    InitializePipeline();
}

ForwardRender::~ForwardRender()
{
    DestroyResources();
}

void ForwardRender::Update(const std::vector<vk::Image>& swapchain_images)
{
    DestroyCommandBuffer();
    DestroyVariablePerImageResources();

    // TODO: release destroy variable resources
    InitializeVariablePerImage(swapchain_images);
    InitCommandBuffer();
}

void ForwardRender::Initialize()
{
    InitCommandBuffer();
}

void ForwardRender::Draw()
{
    auto next_image = m_game.get_device().acquireNextImageKHR(m_game.get_swapchain(), UINT64_MAX, m_sema);

    m_game.get_device().waitForFences(m_swapchain_data[next_image.value].m_fence, VK_TRUE, -1);
    m_game.get_device().resetFences(m_swapchain_data[next_image.value].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_swapchain_data[next_image.value].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(m_sema, stage_flags, command_buffers, m_swapchain_data[next_image.value].m_sema) };
    m_game.get_queue().submit(queue_submits, m_swapchain_data[next_image.value].m_fence);

    std::array wait_sems = { m_swapchain_data[next_image.value].m_sema };

    std::array results{ vk::Result() };
    m_game.get_queue().presentKHR(vk::PresentInfoKHR(wait_sems, m_game.get_swapchain(), next_image.value, results));
}

void ForwardRender::InitializeRenderPass()
{
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_color_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
                                        vk::AttachmentDescription{{}, m_game.get_depth_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };


    vk::AttachmentReference color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depth_attachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::array subpass_description{ vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics,
                                                           {},
                                                           color_attachment,
                                                           {},
                                                           &depth_attachment) };

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

    m_render_pass = m_game.get_device().createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass_description, dependencies));
}

void ForwardRender::DestroyRenderPass()
{
    m_game.get_device().destroyRenderPass(m_render_pass);
}

void ForwardRender::InitializeConstantPerImage()
{
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_fence = m_game.get_device().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_swapchain_data[i].m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());
    }
}

void ForwardRender::DestroyConstantPerImageResources()
{
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_game.get_device().destroyFence(m_swapchain_data[i].m_fence);
        m_game.get_device().destroySemaphore(m_swapchain_data[i].m_sema);
    }
}

void ForwardRender::InitializeVariablePerImage(const std::vector<vk::Image>& swapchain_images)
{
    std::array<uint32_t, 1> queues{ 0 };

    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_color_image = swapchain_images[i];

        m_swapchain_data[i].m_depth_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        m_game.create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

        m_swapchain_data[i].m_color_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        std::array image_views{ m_swapchain_data[i].m_color_image_view, m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_render_pass, image_views, m_game.m_width, m_game.m_height, 1));
    }
}

void ForwardRender::DestroyVariablePerImageResources()
{
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_game.get_device().destroyFramebuffer(m_swapchain_data[i].m_framebuffer);

        m_game.get_device().destroyImageView(m_swapchain_data[i].m_depth_image_view);
        m_game.get_device().destroyImageView(m_swapchain_data[i].m_color_image_view);

        m_game.get_device().freeMemory(m_swapchain_data[i].m_depth_memory);
        m_game.get_device().destroyImage(m_swapchain_data[i].m_depth_image);
    }
}

void ForwardRender::InitializePipeline()
{
    m_vertex_shader = m_game.loadSPIRVShader("Forward.vert.spv");
    m_fragment_shader = m_game.loadSPIRVShader("Forward.frag.spv");

    std::array stages = { vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main"),
                          vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_game.get_layout(), m_render_pass));
    m_pipeline = pipeline_result.value;
}

void ForwardRender::DestroyPipeline()
{
    m_game.get_device().destroyPipeline(m_pipeline);

    m_game.get_device().destroyPipelineCache(m_cache);

    m_game.get_device().destroyShaderModule(m_fragment_shader);
    m_game.get_device().destroyShaderModule(m_vertex_shader);
}

void ForwardRender::InitCommandBuffer()
{
    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    m_command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 2));
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_command_buffer = m_command_buffers[i];

        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());
        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_swapchain_data[i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 0, m_game.get_descriptor_set(), { {} }); /*view_proj_binding*/
        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 3, m_game.get_lights_descriptor_set(), {});

        vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
        m_swapchain_data[i].m_command_buffer.setViewport(0, viewport);
        vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
        m_swapchain_data[i].m_command_buffer.setScissor(0, scissor);

        for (auto& [mat_type, materials] : m_game.get_materials_by_type()) {
            for (auto& material : materials) {
                m_game.get_materials().find(material)->second->UpdateMaterial(m_swapchain_data[i].m_command_buffer);
                for (auto& mesh : m_game.get_mesh_by_material().find(material)->second) {
                    mesh->Draw(m_swapchain_data[i].m_command_buffer);
                }
            }
        }

        m_swapchain_data[i].m_command_buffer.endRenderPass();
        m_swapchain_data[i].m_command_buffer.end();
    }
}

void ForwardRender::DestroyCommandBuffer()
{
    m_game.get_device().freeCommandBuffers(m_game.get_command_pool(), m_command_buffers);
}

void ForwardRender::DestroyResources()
{
    m_game.get_device().waitIdle();
    DestroyCommandBuffer();
    DestroyPipeline();
    DestroyVariablePerImageResources();
    DestroyConstantPerImageResources();
    DestroyRenderPass();
}
