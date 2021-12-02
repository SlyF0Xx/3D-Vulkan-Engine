#include "ForwardRender.h"
#include "Engine.h"
#include "BaseComponents/VulkanComponents/VulkanTransformComponent.h"
#include "BaseComponents/VulkanComponents/VulkanMeshComponent.h"
#include "BaseComponents/VulkanComponents/VulkanCameraComponent.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/VulkanComponents/VulkanDirectionalLightComponent.h"
#include "VulkanInitializer.h"
#include "BaseComponents/DebugComponent.h"

#include "util.h"

ForwardRender::ForwardRender(Game& game, entt::registry& registry)
    : m_game(game), m_registry(registry)
{
    InitializePipelineLayout();
    InitializeRenderPass();

    m_swapchain_data.resize(m_game.get_presentation_engine().m_image_count);
    InitializeConstantPerImage();
    InitializeVariablePerImage();

    InitializePipeline();
    InitializeDebugPipeline();
}

ForwardRender::~ForwardRender()
{
    DestroyResources();
}

void ForwardRender::Update()
{
    DestroyVariablePerImageResources();

    // TODO: release destroy variable resources
    InitializeVariablePerImage();
}

void ForwardRender::Initialize(int i, const vk::CommandBuffer& command_buffer)
{
    InitCommandBuffer(i, command_buffer);
}

void ForwardRender::InitializePipelineLayout()
{
    std::array shadows_bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr), /* directional shadows*/
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr) /* point shadows*/
    };

    m_descriptor_set_layouts = m_game.get_descriptor_set_layouts();
    m_descriptor_set_layouts.push_back(m_game.get_device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, shadows_bindings)));

    std::array push_constants = {
        vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, 0, 4)
    };

    m_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_descriptor_set_layouts, push_constants));

    m_debug_descriptor_set_layouts = {
        m_game.get_descriptor_set_layouts()[0],
        m_game.get_descriptor_set_layouts()[1]
    };

    m_debug_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, m_debug_descriptor_set_layouts));
}

void ForwardRender::InitializeRenderPass()
{
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_color_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, m_game.get_presentation_engine().m_final_layout},
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
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, m_swapchain_data.size()) };
    auto descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, m_swapchain_data.size(), pool_size));

    std::vector<vk::DescriptorSetLayout> layouts(m_swapchain_data.size(), m_descriptor_set_layouts.back());

    auto image_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, layouts));

    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_swapchain_data[i].m_shadows_descriptor_set = image_descriptor_set[i];
    }
}

void ForwardRender::InitializeVariablePerImage()
{
    std::array<uint32_t, 1> queues{ 0 };

    std::vector<vk::WriteDescriptorSet> write_descriptors;
    std::vector<std::vector<vk::DescriptorImageInfo>> shadows_infos;
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        std::array image_views{ m_game.get_presentation_engine().m_swapchain_data[i].m_color_image_view, m_game.get_presentation_engine().m_swapchain_data[i].m_depth_image_view };
        m_swapchain_data[i].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, m_render_pass, image_views, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 1));

        diffusion::VulkanDirectionalLights* light = m_game.get_registry().try_ctx<diffusion::VulkanDirectionalLights>();
        if (light) {
            if (light->m_directional_light_entities.size() == 0) {
                shadows_infos.push_back({ vk::DescriptorImageInfo(light->m_swapchain_data[i].m_point_light_info.m_depth_sampler, light->m_swapchain_data[i].m_point_light_info.m_depth_image_view, vk::ImageLayout::eDepthStencilReadOnlyOptimal) });
            }
            else if (light->m_point_light_entities.size() == 0) {
                // WILL CRASH!
                shadows_infos.push_back({ vk::DescriptorImageInfo(), vk::DescriptorImageInfo(light->m_swapchain_data[i].m_directional_light_info.m_depth_sampler, light->m_swapchain_data[i].m_directional_light_info.m_depth_image_view, vk::ImageLayout::eDepthStencilReadOnlyOptimal) });
            }
            else {
                shadows_infos.push_back({ vk::DescriptorImageInfo(light->m_swapchain_data[i].m_directional_light_info.m_depth_sampler, light->m_swapchain_data[i].m_directional_light_info.m_depth_image_view, vk::ImageLayout::eDepthStencilReadOnlyOptimal),
                                          vk::DescriptorImageInfo(light->m_swapchain_data[i].m_point_light_info.m_depth_sampler, light->m_swapchain_data[i].m_point_light_info.m_depth_image_view, vk::ImageLayout::eDepthStencilReadOnlyOptimal) });
            }
            write_descriptors.push_back(vk::WriteDescriptorSet(m_swapchain_data[i].m_shadows_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, shadows_infos[i], {}, {}));
        }
    }
    m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

void ForwardRender::DestroyVariablePerImageResources()
{
    for (int i = 0; i < m_swapchain_data.size(); ++i) {
        m_game.get_device().destroyFramebuffer(m_swapchain_data[i].m_framebuffer);
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
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eStencilReference /*just compatibility*/ };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_render_pass));
    m_pipeline = pipeline_result.value;
}

void ForwardRender::InitializeDebugPipeline()
{
    m_debug_vertex_shader = m_game.loadSPIRVShader("Debug.vert.spv");
    m_debug_fragment_shader = m_game.loadSPIRVShader("Debug.frag.spv");

    std::array stages = { vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_debug_vertex_shader, "main"),
                          vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_debug_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eLineList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_debug_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_debug_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_render_pass));
    m_debug_pipeline = pipeline_result.value;
}

void ForwardRender::DestroyPipeline()
{
    m_game.get_device().destroyPipeline(m_pipeline);

    m_game.get_device().destroyPipelineCache(m_cache);

    m_game.get_device().destroyShaderModule(m_fragment_shader);
    m_game.get_device().destroyShaderModule(m_vertex_shader);
}

void ForwardRender::InitCommandBuffer(int i, const vk::CommandBuffer & command_buffer)
{
    std::array clear_values = {
        vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
        vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
    };

    diffusion::VulkanDirectionalLights* light = m_game.get_registry().try_ctx<diffusion::VulkanDirectionalLights>();
    if (light) {
        diffusion::VulkanInitializer::init_command_buffer(m_game, *light, i, command_buffer);
    }

    command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_swapchain_data[i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height)), clear_values), vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    const auto * main_camera_component = m_registry.try_ctx<diffusion::MainCameraTag>();
    if (main_camera_component) {
        const auto& camera = m_registry.get<const diffusion::VulkanCameraComponent>(main_camera_component->m_entity);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, camera.m_descriptor_set, { {} });
    }

    if (light) {
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 3, light->m_lights_descriptor_set, {});
    }
    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 4, m_swapchain_data[i].m_shadows_descriptor_set, {});

    vk::Viewport viewport(0, 0, m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height, 0.0f, 1.0f);
    command_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.get_presentation_engine().m_width, m_game.get_presentation_engine().m_height));
    command_buffer.setScissor(0, scissor);

    {
        int unlit = 1;
        command_buffer.pushConstants(m_layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 0);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 0);

        auto unlit_view = m_registry.view<const diffusion::UnlitMaterialComponent,
            const diffusion::VulkanTransformComponent,
            const diffusion::VulkanSubMesh,
            const diffusion::SubMesh>(entt::exclude<diffusion::debug_tag>);

        ::entt::entity material_entity{ ::entt::null };

        unlit_view.each([this, i, &material_entity, &command_buffer](
            const diffusion::UnlitMaterialComponent& unlit,
            const diffusion::VulkanTransformComponent& transform,
            const diffusion::VulkanSubMesh& vulkan_mesh,
            const diffusion::SubMesh& mesh) {

                if (unlit.m_reference != material_entity) {
                    material_entity = unlit.m_reference;
                    const auto& unlit_material = m_registry.get<const diffusion::UnlitMaterial>(unlit.m_reference);
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 2, unlit_material.m_descriptor_set, { });
                }

                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 1, transform.m_descriptor_set, { {} });

                command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
                command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
                command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
            });
    }

    {
        int unlit = 0;
        command_buffer.pushConstants(m_layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 1);
        command_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 1);

        auto func = m_game.get_device().getProcAddr("vkCmdSetPrimitiveTopologyEXT");
        reinterpret_cast<PFN_vkCmdSetPrimitiveTopologyEXT>(static_cast<VkCommandBuffer>(command_buffer), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //command_buffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);

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
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 2, lit_material.m_descriptor_set, { });
                }

                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 1, transform.m_descriptor_set, { {} });

                command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
                command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
                command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
            });
    }








    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_debug_pipeline);

    if (main_camera_component) {
        const auto& camera = m_registry.get<const diffusion::VulkanCameraComponent>(main_camera_component->m_entity);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_debug_layout, 0, camera.m_descriptor_set, { {} });
    }



    {
        auto unlit_view = m_registry.view<const diffusion::VulkanTransformComponent,
            const diffusion::VulkanSubMesh,
            const diffusion::SubMesh,
            const diffusion::debug_tag>();

        unlit_view.each([this, i, &command_buffer](
            const diffusion::VulkanTransformComponent& transform,
            const diffusion::VulkanSubMesh& vulkan_mesh,
            const diffusion::SubMesh& mesh) {
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_debug_layout, 1, transform.m_descriptor_set, { {} });

                command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
                command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
                command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
            });
    }



    command_buffer.endRenderPass();
}

void ForwardRender::DestroyResources()
{
    m_game.get_device().waitIdle();
    DestroyPipeline();
    DestroyVariablePerImageResources();
    DestroyRenderPass();
}
