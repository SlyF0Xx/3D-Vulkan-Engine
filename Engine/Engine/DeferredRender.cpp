#include "DeferredRender.h"
#include "Engine.h"

#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"
#include "Entity.h"
#include "VulkanMeshComponentManager.h"

DeferredRender::DeferredRender(Game& game, const std::vector<vk::Image>& swapchain_images)
    : m_game(game)
{
    m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());
    
    InitializeDeferredPipelineLayout();
    InitializeCompositePipelineLayout();
    InitializeDeferredRenderPass();
    InitializeCompositeRenderPass();

    m_swapchain_data.resize(swapchain_images.size());
    InitializeConstantPerImage();
    InitializeVariablePerImage(swapchain_images);

    InitializeDeferredPipeline();
    InitializeCompositePipeline();
}

void DeferredRender::Update(const std::vector<vk::Image>& swapchain_images)
{
    // TODO: release destroy variable resources
    InitializeVariablePerImage(swapchain_images);
    InitCommandBuffer();
}

void DeferredRender::Initialize()
{
    InitCommandBuffer();
}

void DeferredRender::Draw()
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

void DeferredRender::InitializeDeferredPipelineLayout()
{
    // for comatibility
    std::array push_constants = {
        vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, 0, 4)
    };
    m_deferred_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_game.get_descriptor_set_layouts(), push_constants));
}

void DeferredRender::InitializeCompositePipelineLayout()
{
    std::array material_bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr), /*albedo*/
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr), /*normal*/
        vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr), /*depth*/
        vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr) /*shadows*/
    };

    //m_game.get_descriptor_set_layouts()[2], /*albedo + normal*/

    m_composite_descriptor_set_layouts = {
        m_game.get_device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, material_bindings)),
        m_game.get_descriptor_set_layouts()[3]
    };

    std::array push_constants = {
        vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, 0, 4)
    };

    m_composite_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_composite_descriptor_set_layouts, push_constants));
}

void DeferredRender::InitializeDeferredRenderPass()
{
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_color_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal},
                                        vk::AttachmentDescription{{}, m_game.get_color_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal},
                                        vk::AttachmentDescription{{}, m_game.get_depth_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };


    std::array color_attachments{ vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
                                  vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal) };
    vk::AttachmentReference depth_attachment(2, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::array subpass_description{ vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics,
                                                           {},
                                                           color_attachments,
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

    m_deffered_render_pass = m_game.get_device().createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass_description, dependencies));
}

void DeferredRender::InitializeCompositeRenderPass()
{
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

    std::array game_component_attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_color_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR},
                                                       vk::AttachmentDescription{{}, m_game.get_depth_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal} };

    m_composite_render_pass = m_game.get_device().createRenderPass(vk::RenderPassCreateInfo({}, game_component_attachment_descriptions, subpass_description, dependencies));

}

void DeferredRender::InitializeConstantPerImage()
{
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 4) };
    auto descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 4, pool_size));

    std::array layouts{
        m_composite_descriptor_set_layouts[0],
        m_composite_descriptor_set_layouts[0]
    };

    auto image_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, layouts));
    /*
    std::array depth_layouts{
        m_composite_descriptor_set_layouts[2],
        m_composite_descriptor_set_layouts[2]
    };

    auto depth_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, depth_layouts));
    */
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_descriptor_set = image_descriptor_set[i];
        //m_swapchain_data[i].m_depth_descriptor_set = depth_descriptor_set[i];

        m_swapchain_data[i].m_fence = m_game.get_device().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_swapchain_data[i].m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());
    }
}

void DeferredRender::InitializeVariablePerImage(const std::vector<vk::Image>& swapchain_images)
{
    std::array<uint32_t, 1> queues{ 0 };

    std::vector<vk::WriteDescriptorSet> write_descriptors;
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_color_image = swapchain_images[i];
        m_swapchain_data[i].m_deffered_depth_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        m_game.create_memory_for_image(m_swapchain_data[i].m_deffered_depth_image, m_swapchain_data[i].m_deffered_depth_memory);

        m_swapchain_data[i].m_depth_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        m_game.create_memory_for_image(m_swapchain_data[i].m_depth_image, m_swapchain_data[i].m_depth_memory);

        m_swapchain_data[i].m_albedo_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_color_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        m_game.create_memory_for_image(m_swapchain_data[i].m_albedo_image, m_swapchain_data[i].m_albedo_memory);

        m_swapchain_data[i].m_normal_image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_color_format(), vk::Extent3D(m_game.m_width, m_game.m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
        m_game.create_memory_for_image(m_swapchain_data[i].m_normal_image, m_swapchain_data[i].m_normal_memory);

        m_swapchain_data[i].m_albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_albedo_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_normal_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_normal_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_color_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_color_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_depth_image, vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
        m_swapchain_data[i].m_deffered_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_deffered_depth_image, vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));

        m_swapchain_data[i].m_albedo_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
        
        std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_swapchain_data[i].m_albedo_sampler, m_swapchain_data[i].m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}));


        m_swapchain_data[i].m_normal_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

        std::array normal_descriptor_image_infos{ vk::DescriptorImageInfo(m_swapchain_data[i].m_normal_sampler, m_swapchain_data[i].m_normal_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 1, 0, vk::DescriptorType::eCombinedImageSampler, normal_descriptor_image_infos, {}, {}));


        m_swapchain_data[i].m_depth_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

        m_swapchain_data[i].m_deffered_depth_image_view_stencil_only = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_deffered_depth_image, vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)));
        std::array depth_descriptor_image_infos{ vk::DescriptorImageInfo(m_swapchain_data[i].m_depth_sampler, m_swapchain_data[i].m_deffered_depth_image_view_stencil_only, vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 2/*m_swapchain_data[i].m_depth_descriptor_set, 0*/, 0, vk::DescriptorType::eCombinedImageSampler, depth_descriptor_image_infos, {}, {}));

        std::array shadows_infos{ vk::DescriptorImageInfo(m_game.get_shadpwed_lights().get_depth_sampler(i), m_game.get_shadpwed_lights().get_depth_image_view(i), vk::ImageLayout::eShaderReadOnlyOptimal) };
        write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 3, 0, vk::DescriptorType::eCombinedImageSampler, shadows_infos, {}, {}));

        std::array deffered_views{ m_swapchain_data[i].m_albedo_image_view, m_swapchain_data[i].m_normal_image_view, m_swapchain_data[i].m_deffered_depth_image_view };
        m_swapchain_data[i].m_deffered_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_deffered_render_pass, deffered_views, m_game.m_width, m_game.m_height, 1));

        std::array image_views{ m_swapchain_data[i].m_color_image_view, m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_composite_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_composite_render_pass, image_views, m_game.m_width, m_game.m_height, 1));
    }
    m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

void DeferredRender::InitializeDeferredPipeline()
{
    //E:\\programming\\Graphics\\Game\\Engine\\Engine\\

    m_deffered_vertex_shader = m_game.loadSPIRVShader("Deffered.vert.spv");
    m_deffered_fragment_shader = m_game.loadSPIRVShader("Deffered.frag.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_deffered_vertex_shader, "main"),
                       vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_deffered_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_TRUE,
                                                               vk::StencilOpState(vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::CompareOp::eAlways, 0xFF, 0xFF, 0xFF),
                                                               vk::StencilOpState(vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::CompareOp::eAlways, 0xFF, 0xFF, 0xFF),
                                                               0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE),
                      vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    blend[1].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_deffered_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eStencilReference };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_deffered_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_deferred_layout, m_deffered_render_pass));
    m_deffered_pipeline = pipeline_result.value;
}

void DeferredRender::InitializeCompositePipeline()
{
    m_composite_vertex_shader = m_game.loadSPIRVShader("Composite.vert.spv");
    m_composite_fragment_shader = m_game.loadSPIRVShader("Composite.frag.spv");

    std::array stages = { vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_composite_vertex_shader, "main"),
               vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_composite_fragment_shader, "main")
    };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
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

    m_composite_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_composite_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_composite_layout, m_composite_render_pass));
    m_composite_pipeline = pipeline_result.value;
}

void DeferredRender::InitCommandBuffer()
{
    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                       vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    auto command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 2));
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_command_buffer = command_buffers[i];

        m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

        for (int j = 0; j < m_game.get_shadpwed_lights().get_shadowed_light().size(); ++j) {
            m_game.get_shadpwed_lights().get_shadowed_light()[j].InitCommandBuffer(i, m_swapchain_data[i].m_command_buffer);
        }

        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_deffered_render_pass, m_swapchain_data[i].m_deffered_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_deffered_pipeline);

        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 0, m_game.get_descriptor_set(), { {} });

        vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
        m_swapchain_data[i].m_command_buffer.setViewport(0, viewport);
        vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
        m_swapchain_data[i].m_command_buffer.setScissor(0, scissor);

        for (auto& [mat_type, materials] : diffusion::s_vulkan_mesh_component_manager.get_materials_by_type()) {
            for (auto& material : materials) {
                diffusion::s_vulkan_mesh_component_manager.get_materials().find(material)->second->UpdateMaterial(m_deferred_layout, m_swapchain_data[i].m_command_buffer);


                for (auto& mesh_component : diffusion::s_vulkan_mesh_component_manager.get_mesh_by_material().find(material)->second) {
                    diffusion::Entity* parent = mesh_component->get_parrent();
                    for (auto& inner_component : parent->get_components()) {
                        auto it = std::find(
                            inner_component.get().get_tags().begin(),
                            inner_component.get().get_tags().end(),
                            diffusion::VulkanTransformComponent::s_vulkan_transform_component_tag);
                        if (it != inner_component.get().get_tags().end()) {
                            auto comp = dynamic_cast<diffusion::VulkanTransformComponent&>(inner_component.get());
                            comp.Draw(m_deferred_layout, m_swapchain_data[i].m_command_buffer);
                        }
                    }

                    auto comp = dynamic_cast<diffusion::VulkanMeshComponent*>(mesh_component);
                    comp->Draw(m_swapchain_data[i].m_command_buffer);
                }
                /*
                for (auto& mesh : m_game.get_mesh_by_material().find(material)->second) {
                    mesh->Draw(m_deferred_layout, m_swapchain_data[i].m_command_buffer);
                }
                */


            }
        }

        m_swapchain_data[i].m_command_buffer.endRenderPass();

        m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_composite_render_pass, m_swapchain_data[i].m_composite_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), colors), vk::SubpassContents::eInline);
        m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_composite_pipeline);

        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 0, m_swapchain_data[i].m_descriptor_set, {});
        m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 1, m_game.get_lights_descriptor_set(), {});
        //m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 2, m_swapchain_data[i].m_depth_descriptor_set, {});
        m_swapchain_data[i].m_command_buffer.draw(3, 1, 0, 0);
        m_swapchain_data[i].m_command_buffer.endRenderPass();
        m_swapchain_data[i].m_command_buffer.end();
    }
}
