#include "ShadowMap.h"

#include "Engine.h"
#include "util.h"
#include "VulkanMeshComponent.h"
#include "VulkanTransformComponent.h"

#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"


ShadowMap::ShadowMap(
    Game& game,
    const glm::vec3& position,
    const glm::vec3& cameraTarget,
    const glm::vec3& upVector,
    const glm::mat4& ProjectionMatrix,
    int light_index,
    const std::vector<vk::Image>& swapchain_images)
    : m_game(game), m_position(position), m_cameraTarget(cameraTarget), m_upVector(upVector), m_ProjectionMatrix(ProjectionMatrix)
{
    InitializeCompositePipelineLayout();
    InitializeDescriptorSet();
    InitializeDeferredRenderPass();

    m_swapchain_data.resize(swapchain_images.size());
    InitializeVariablePerImage(light_index, swapchain_images);

    InitializeDeferredPipeline();
}

void ShadowMap::InitializeCompositePipelineLayout()
{
    m_descriptor_set_layouts = {
        m_game.get_descriptor_set_layouts()[0],
        m_game.get_descriptor_set_layouts()[1]
    };

    m_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_descriptor_set_layouts, {}));
}

void ShadowMap::InitializeDescriptorSet()
{
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 2) };
    auto descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 2, pool_size));

    m_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_descriptor_set_layouts[0]))[0];

    m_CameraMatrix = glm::lookAt(
        m_position, // Позиция камеры в мировом пространстве
        m_cameraTarget,   // Указывает куда вы смотрите в мировом пространстве
        m_upVector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );
    m_view_projection_matrix = m_ProjectionMatrix * m_CameraMatrix;

    std::vector matrixes{ m_view_projection_matrix };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
    m_world_view_projection_matrix_buffer = out2.m_buffer;
    m_world_view_projection_matrix_memory = out2.m_allocation;
    m_world_view_projection_mapped_memory = out2.m_mapped_memory;

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_view_projection_matrix_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

void ShadowMap::InitializeDeferredRenderPass()
{
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_depth_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilReadOnlyOptimal} };

    vk::AttachmentReference depth_attachment(0, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    
    /*
    std::array subpass_description{ vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics,
                                                           {},
                                                           {},
                                                           {},
                                                           &depth_attachment) };
    */
    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics);
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depth_attachment;

    std::array dependencies{ vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                               vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                               vk::AccessFlagBits::eShaderRead,
                                               vk::AccessFlagBits::eDepthStencilAttachmentWrite/*,
                                               vk::DependencyFlagBits::eByRegion*/),
                         vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL,
                                               vk::PipelineStageFlagBits::eLateFragmentTests,
                                               vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                               vk::AccessFlagBits::eShaderRead/*,
                                               vk::DependencyFlagBits::eByRegion*/)
    };
    
    m_render_pass = m_game.get_device().createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass, dependencies));
}

void ShadowMap::DestroyVariablePerImage()
{
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_game.get_device().destroyImageView(m_swapchain_data[i].m_depth_image_view);
        m_game.get_device().destroyFramebuffer(m_swapchain_data[i].m_framebuffer);
    }
}

void ShadowMap::InitializeVariablePerImage(int light_index, const std::vector<vk::Image>& swapchain_images)
{
    std::array<uint32_t, 1> queues{ 0 };

    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, swapchain_images[i], vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, light_index, 1)));

        std::array deffered_views{ m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_render_pass, deffered_views, m_game.m_width, m_game.m_height, 1));
    }
}

void ShadowMap::InitializeDeferredPipeline()
{
    //E:\\programming\\Graphics\\Game\\Engine\\Engine\\

    m_vertex_shader = m_game.loadSPIRVShader("ShadowMap.vert.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f) }; /* TODO: shadow map resolution */
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, {});

    m_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_render_pass));
    m_pipeline = pipeline_result.value;
}

void ShadowMap::InitCommandBuffer(int index, const vk::CommandBuffer& command_buffer)
{
    std::array colors{ vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0)) };

    command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_swapchain_data[index].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), colors), vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, m_descriptor_set, { {} });

    vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
    command_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
    command_buffer.setScissor(0, scissor);

    auto view = m_game.get_registry().view<
        const diffusion::VulkanTransformComponent,
        const diffusion::VulkanSubMesh,
        const diffusion::SubMesh>();

    view.each([this, &command_buffer](
        const diffusion::VulkanTransformComponent& transform,
        const diffusion::VulkanSubMesh& vulkan_mesh,
        const diffusion::SubMesh& mesh) {
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 1, transform.m_descriptor_set, { {} });

        command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
        command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
        command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
    });

    command_buffer.endRenderPass();
}

void ShadowMap::ResetImages(int light_index, const std::vector<vk::Image>& swapchain_images)
{
    DestroyVariablePerImage();
    m_swapchain_data.resize(swapchain_images.size());
    InitializeVariablePerImage(light_index, swapchain_images);
}

glm::vec3 ShadowMap::get_direction()
{
    return m_cameraTarget - m_position;
}

glm::mat4 ShadowMap::get_view_proj_matrix()
{
    return m_ProjectionMatrix * m_CameraMatrix;
}
