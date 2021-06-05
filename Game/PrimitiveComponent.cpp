#include "PrimitiveComponent.h"

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#if 0
PrimitiveComponent::PrimitiveComponent(Game& game, const std::vector<PrimitiveVertex> & verticies)
    : m_game(game), m_verticies(verticies)
{}

void PrimitiveComponent::Initialize(std::size_t num_of_swapchain_images)
{
    m_command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 2));

    std::array<uint32_t, 1> queues{ 0 };
    m_infos.resize(num_of_swapchain_images);

    m_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, {}, {}));

    m_vertex_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\Prim.vert.spv");
    m_fragment_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\Prim.frag.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main"),
                       vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);
    // TODO dynamic viewport and scissors

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_FALSE /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_game.get_game_component_render_pass()));
    m_pipeline = pipeline_result.value;





    /*m_verticies = { PrimitiveVertex{0.25f, 0.75f, 0.5f},
                    PrimitiveVertex{0.5f,  0.25f, 0.5f},
                    PrimitiveVertex{0.75f, 0.75f, 0.5f}
    };*/

    std::size_t buffer_size = m_verticies.size() * sizeof(PrimitiveVertex);
    m_vertex_buffer = m_game.get_device().createBuffer(vk::BufferCreateInfo({}, buffer_size, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive, queues));
    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_vertex_buffer);

    uint32_t buffer_index = m_game.find_appropriate_memory_type(memory_buffer_req, m_game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_vertex_memory = m_game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

    void* mapped_data = nullptr/*= malloc(buffer_size)*/;
    m_game.get_device().mapMemory(m_vertex_memory, {}, memory_buffer_req.size, {}, &mapped_data);
    std::memcpy(mapped_data, m_verticies.data(), buffer_size);

    m_game.get_device().bindBufferMemory(m_vertex_buffer, m_vertex_memory, {});
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_vertex_memory, {}, memory_buffer_req.size));



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

    for (int mat_i = 0; mat_i < num_of_swapchain_images; ++mat_i) {
        m_infos[mat_i].m_fence = m_game.get_device().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_infos[mat_i].m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());

        m_infos[mat_i].m_command_buffer = m_command_buffers[mat_i];
        init_cmd_buffer(m_infos[mat_i].m_command_buffer, mat_i);
    }
}

void PrimitiveComponent::init_cmd_buffer(const vk::CommandBuffer& cmd_buffer, int index)
{
    cmd_buffer.begin(vk::CommandBufferBeginInfo());

    cmd_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_game.get_game_component_render_pass(), m_game.get_swapchain_data()[index].m_game_component_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), {}), vk::SubpassContents::eInline);

    cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
    cmd_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
    cmd_buffer.setScissor(0, scissor);

    // m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game_impl->m_materials[mat_i].m_layout, 0, m_game_impl->m_materials[mat_i].m_descriptor_sets, {});

    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.draw(m_verticies.size(), m_verticies.size() / 3, 0, 0);

    cmd_buffer.endRenderPass();
    cmd_buffer.end();
}

void PrimitiveComponent::Update(std::size_t num_of_swapchain_images, int width, int height)
{
    for (int mat_i = 0; mat_i < num_of_swapchain_images; ++mat_i) {
        m_command_buffers[mat_i].reset();
        init_cmd_buffer(m_command_buffers[mat_i], mat_i);
    }
}

vk::Semaphore PrimitiveComponent::Draw(int swapchain_image_index, vk::Semaphore wait_sema)
{
    m_game.get_device().waitForFences(m_infos[swapchain_image_index].m_fence, VK_TRUE, -1);
    m_game.get_device().resetFences(m_infos[swapchain_image_index].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_infos[swapchain_image_index].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(wait_sema, stage_flags, command_buffers, m_infos[swapchain_image_index].m_sema) };
    m_game.get_queue().submit(queue_submits, m_infos[swapchain_image_index].m_fence);

    return m_infos[swapchain_image_index].m_sema;
}

void PrimitiveComponent::DestroyResources()
{
    m_game.get_device().freeCommandBuffers(m_game.get_command_pool(), m_command_buffers);
}

#endif