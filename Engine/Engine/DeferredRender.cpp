#include "DeferredRender.h"
#include "Engine.h"
#include "BaseComponents/VulkanComponents/VulkanTransformComponent.h"
#include "BaseComponents/VulkanComponents/VulkanMeshComponent.h"
#include "BaseComponents/VulkanComponents/VulkanCameraComponent.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/LitMaterial.h"

DeferredRender::DeferredRender(Game& game)
    : m_game(game), m_registry(m_game.get_registry())
{
    InitializeDeferredPipelineLayout();
    InitializeCompositePipelineLayout();
    InitializeDeferredRenderPass();
    InitializeCompositeRenderPass();

    m_swapchain_data.resize(m_game.get_presentation_engine().m_image_count);
    InitializeConstantPerImage();
    InitializeVariablePerImage();

    InitializeDeferredPipeline();
    InitializeCompositePipeline();
}

void DeferredRender::Update()
{
    // TODO: release destroy variable resources
    InitializeVariablePerImage();
    //InitCommandBuffer();
}

void DeferredRender::Initialize(int i, const vk::CommandBuffer& command_buffer)
{
    InitCommandBuffer(i, command_buffer);
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

    std::vector layouts(m_swapchain_data.size(), m_composite_descriptor_set_layouts[0]);

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
    }
}

void DeferredRender::InitializeVariablePerImage()
{
    std::array<uint32_t, 1> queues{ 0 };

    std::vector<vk::WriteDescriptorSet> write_descriptors;
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        auto deffered_allocation = m_game.get_allocator().createImage(
            vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_depth_format(), vk::Extent3D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
            vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
        m_swapchain_data[i].m_deffered_depth_image = deffered_allocation.first;
        m_swapchain_data[i].m_deffered_depth_memory = deffered_allocation.second;

        auto albedo_allocation = m_game.get_allocator().createImage(
            vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_color_format(), vk::Extent3D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
            vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
        m_swapchain_data[i].m_albedo_image = albedo_allocation.first;
        m_swapchain_data[i].m_albedo_memory = albedo_allocation.second;

        auto normal_allocation = m_game.get_allocator().createImage(
            vk::ImageCreateInfo({}, vk::ImageType::e2D, m_game.get_color_format(), vk::Extent3D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
            vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));
        m_swapchain_data[i].m_normal_image = normal_allocation.first;
        m_swapchain_data[i].m_normal_memory = normal_allocation.second;

        m_swapchain_data[i].m_albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_albedo_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        m_swapchain_data[i].m_normal_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_swapchain_data[i].m_normal_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
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

        diffusion::VulkanDirectionalLights* light = m_game.get_registry().try_ctx<diffusion::VulkanDirectionalLights>();
        if (light) {
            std::array shadows_infos{ vk::DescriptorImageInfo(light->m_swapchain_data[i].m_directional_light_info.m_depth_sampler, light->m_swapchain_data[i].m_directional_light_info.m_depth_image_view, vk::ImageLayout::eShaderReadOnlyOptimal),
                                      vk::DescriptorImageInfo(light->m_swapchain_data[i].m_point_light_info.m_depth_sampler, light->m_swapchain_data[i].m_point_light_info.m_depth_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
            write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_descriptor_set, 3, 0, vk::DescriptorType::eCombinedImageSampler, shadows_infos, {}, {}));
        }

        std::array deffered_views{ m_swapchain_data[i].m_albedo_image_view, m_swapchain_data[i].m_normal_image_view, m_swapchain_data[i].m_deffered_depth_image_view };
        m_swapchain_data[i].m_deffered_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_deffered_render_pass, deffered_views, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1));

        std::array image_views{ m_game.get_presentation_engine().m_swapchain_data[i].m_color_image_view, m_game.get_presentation_engine().m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_composite_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_composite_render_pass, image_views, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1));
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

    std::array viewports{ vk::Viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)) };
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

    std::array viewports{ vk::Viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)) };
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

void DeferredRender::InitCommandBuffer(int i, const vk::CommandBuffer& command_buffer)
{
    std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                       vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f })),
                       vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    diffusion::VulkanDirectionalLights* light = m_game.get_registry().try_ctx<diffusion::VulkanDirectionalLights>();
    if (light) {
        diffusion::VulkanInitializer::init_command_buffer(m_game, *light, i, command_buffer);
    }

    command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_deffered_render_pass, m_swapchain_data[i].m_deffered_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)), colors), vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_deffered_pipeline);

    const auto* main_camera_component = m_registry.try_ctx<diffusion::MainCameraTag>();
    if (main_camera_component) {
        const auto& camera = m_registry.get<const diffusion::VulkanCameraComponent>(main_camera_component->m_entity);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 0, camera.m_descriptor_set, { {} });
    }

    vk::Viewport viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f);
    command_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height));
    command_buffer.setScissor(0, scissor);


    {
        int unlit = 1;
        command_buffer.pushConstants(m_deferred_layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 0);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 0);

        auto unlit_view = m_registry.view<const diffusion::UnlitMaterialComponent,
            const diffusion::VulkanTransformComponent,
            const diffusion::VulkanSubMesh,
            const diffusion::SubMesh>();

        ::entt::entity material_entity{ ::entt::null };

        unlit_view.each([this, i, &material_entity, &command_buffer](
            const diffusion::UnlitMaterialComponent& unlit,
            const diffusion::VulkanTransformComponent& transform,
            const diffusion::VulkanSubMesh& vulkan_mesh,
            const diffusion::SubMesh& mesh) {

            if (unlit.m_reference != material_entity) {
                material_entity = unlit.m_reference;
                const auto& unlit_material = m_registry.get<const diffusion::UnlitMaterial>(unlit.m_reference);
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 2, unlit_material.m_descriptor_set, { });
            }

            command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 1, transform.m_descriptor_set, { {} });

            command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
            command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
            command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
        });
    }


    {
        int unlit = 0;
        command_buffer.pushConstants(m_deferred_layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 1);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 1);

        auto lit_view = m_registry.view<const diffusion::LitMaterialComponent,
            const diffusion::VulkanTransformComponent,
            const diffusion::VulkanSubMesh,
            const diffusion::SubMesh>();

        ::entt::entity material_entity = ::entt::null;

        lit_view.each([this, i, &material_entity, &command_buffer](
            const diffusion::LitMaterialComponent& lit,
            const diffusion::VulkanTransformComponent& transform,
            const diffusion::VulkanSubMesh& vulkan_mesh,
            const diffusion::SubMesh& mesh) {

            if (lit.m_reference != material_entity) {
                material_entity = lit.m_reference;
                const auto& lit_material = m_registry.get<const diffusion::LitMaterial>(lit.m_reference);
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 2, lit_material.m_descriptor_set, { });
            }

            command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_deferred_layout, 1, transform.m_descriptor_set, { {} });

            command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
            command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
            command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
        });
    }

    command_buffer.endRenderPass();

    command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_composite_render_pass, m_swapchain_data[i].m_composite_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)), colors), vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_composite_pipeline);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 0, m_swapchain_data[i].m_descriptor_set, {});

    if (light) {
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 1, light->m_lights_descriptor_set, {});
    }
    //command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_composite_layout, 2, m_swapchain_data[i].m_depth_descriptor_set, {});
    command_buffer.draw(3, 1, 0, 0);
    command_buffer.endRenderPass();
}
