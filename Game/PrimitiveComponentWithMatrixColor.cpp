#include "PrimitiveComponentWithMatrixColor.h"
#include "util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

PrimitiveComponentWithMatrixColor::PrimitiveComponentWithMatrixColor(
    Game& game,
    const std::vector<PrimitiveColoredVertex>& verticies,
    const std::vector<uint32_t>& indexes,
    const BoundingSphere& bounding_sphere,
    const glm::vec3 & position,
    const glm::vec3 & rotation,
    const glm::vec3 & scale,
    const glm::mat4& CameraMatrix,
    const glm::mat4& ProjectionMatrix)
    : m_game(game), m_verticies(verticies), m_indexes(indexes), m_bounding_sphere(bounding_sphere)
{
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1), position);
    
    glm::mat4 rotation_matrix(1);

    glm::vec3 RotationX(1.0, 0, 0);
    glm::rotate(rotation_matrix, rotation[0], RotationX);

    glm::vec3 RotationY(0, 1.0, 0);
    glm::rotate(rotation_matrix, rotation[1], RotationY);

    glm::vec3 RotationZ(0, 0, 1.0);
    glm::rotate(rotation_matrix, rotation[2], RotationZ);

    glm::mat4 scale_matrix = glm::scale(scale);

    m_world_matrix = translation_matrix * rotation_matrix * scale_matrix;
    m_view_matrix = CameraMatrix;
    m_projection_matrix = ProjectionMatrix;
    m_world_view_projection_matrix = m_projection_matrix * m_view_matrix * m_world_matrix;
}

glm::mat4 PrimitiveComponentWithMatrixColor::get_world_matrix()
{
    return m_world_matrix;
}

void PrimitiveComponentWithMatrixColor::UpdateWorldMatrix(const glm::mat4& world_matrix)
{
    m_world_matrix = world_matrix;
    m_world_view_projection_matrix = m_projection_matrix * m_view_matrix * m_world_matrix;
    std::vector matrixes{ m_world_view_projection_matrix };
    
    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);
    
    void* mapped_data = nullptr;
    m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
    std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
    m_game.get_device().unmapMemory(m_world_matrix_memory);
    
    
    /*
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_world_matrix_buffer = out2.m_buffer;
    m_world_matrix_memory = out2.m_memory;
    */
    m_game.get_device().waitIdle();

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    for (int mat_i = 0; mat_i < 2; ++mat_i) {
        m_command_buffers[mat_i].reset();
        init_cmd_buffer(m_command_buffers[mat_i], mat_i);
    }
}

void PrimitiveComponentWithMatrixColor::UpdateViewMatrix(const glm::mat4& view_matrix)
{
    m_view_matrix = view_matrix;
    m_world_view_projection_matrix = m_projection_matrix * m_view_matrix * m_world_matrix;
    std::vector matrixes{ m_world_view_projection_matrix };

    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);

    void* mapped_data = nullptr;
    m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
    std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
    m_game.get_device().unmapMemory(m_world_matrix_memory);


    /*
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_world_matrix_buffer = out2.m_buffer;
    m_world_matrix_memory = out2.m_memory;
    */
    m_game.get_device().waitIdle();

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    for (int mat_i = 0; mat_i < 2; ++mat_i) {
        m_command_buffers[mat_i].reset();
        init_cmd_buffer(m_command_buffers[mat_i], mat_i);
    }
}

void PrimitiveComponentWithMatrixColor::SetWVPMatrix(const glm::mat4& world_view_projection_matrix)
{
    m_world_view_projection_matrix = world_view_projection_matrix;
    std::vector matrixes{ m_world_view_projection_matrix };
    
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_world_matrix_buffer = out2.m_buffer;
    m_world_matrix_memory = out2.m_memory;

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

void PrimitiveComponentWithMatrixColor::Initialize(std::size_t num_of_swapchain_images)
{
    m_command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 2));

    std::array<uint32_t, 1> queues{ 0 };
    m_infos.resize(num_of_swapchain_images);

    std::array bindings{
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
    };

    std::array set_layouts{
        m_game.get_device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings))
    };
    m_layout = m_game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, set_layouts, {}));

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, set_layouts))[0];

    std::vector matrixes{ m_world_view_projection_matrix };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_world_matrix_buffer = out2.m_buffer;
    m_world_matrix_memory = out2.m_memory;




    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});


    m_vertex_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\ColorMatrix.vert.spv");
    m_fragment_shader = m_game.loadSPIRVShader("E:\\programming\\Graphics\\Game\\Engine\\Engine\\ColorMatrix.frag.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_vertex_shader, "main"),
                       vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_fragment_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f) };
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);
    // TODO dynamic viewport and scissors

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone /*eFront*/, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_TRUE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;

    std::array blend{ vk::PipelineColorBlendAttachmentState(VK_FALSE) };
    blend[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, blend);

    m_cache = m_game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = m_game.get_device().createGraphicsPipeline(m_cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, m_layout, m_game.get_game_component_render_pass()));
    m_pipeline = pipeline_result.value;




    auto out = create_buffer(m_game, m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    m_vertex_buffer = out.m_buffer;
    m_vertex_memory = out.m_memory;

    auto out3 = create_buffer(m_game, m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);
    m_index_buffer = out3.m_buffer;
    m_index_memory = out3.m_memory;
    /*
    std::size_t buffer_size = m_verticies.size() * sizeof(PrimitiveColoredVertex);
    m_vertex_buffer = m_game.get_device().createBuffer(vk::BufferCreateInfo({}, buffer_size, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive, queues));
    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_vertex_buffer);

    uint32_t buffer_index = m_game.find_appropriate_memory_type(memory_buffer_req, m_game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_vertex_memory = m_game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

    void* mapped_data = nullptr;
    m_game.get_device().mapMemory(m_vertex_memory, {}, memory_buffer_req.size, {}, &mapped_data);
    std::memcpy(mapped_data, m_verticies.data(), buffer_size);

    m_game.get_device().bindBufferMemory(m_vertex_buffer, m_vertex_memory, {});
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_vertex_memory, {}, memory_buffer_req.size));
    */




    for (int mat_i = 0; mat_i < num_of_swapchain_images; ++mat_i) {
        m_infos[mat_i].m_fence = m_game.get_device().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_infos[mat_i].m_sema = m_game.get_device().createSemaphore(vk::SemaphoreCreateInfo());

        m_infos[mat_i].m_command_buffer = m_command_buffers[mat_i];
        init_cmd_buffer(m_infos[mat_i].m_command_buffer, mat_i);
    }
}

void PrimitiveComponentWithMatrixColor::init_cmd_buffer(const vk::CommandBuffer& cmd_buffer, int index)
{
    cmd_buffer.begin(vk::CommandBufferBeginInfo());

    cmd_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_game.get_game_component_render_pass(), m_game.get_swapchain_data()[index].m_game_component_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), {}), vk::SubpassContents::eInline);

    cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
    cmd_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
    cmd_buffer.setScissor(0, scissor);

    //cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, m_descriptor_set, {});
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, m_descriptor_set, { {} });

    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.bindIndexBuffer(m_index_buffer, {}, vk::IndexType::eUint32);
    cmd_buffer.drawIndexed(m_indexes.size(), m_indexes.size() / 3, 0, 0, 0);

    cmd_buffer.endRenderPass();
    cmd_buffer.end();
}

void PrimitiveComponentWithMatrixColor::Update(std::size_t num_of_swapchain_images, int width, int height)
{
    for (int mat_i = 0; mat_i < num_of_swapchain_images; ++mat_i) {
        m_command_buffers[mat_i].reset();
        init_cmd_buffer(m_command_buffers[mat_i], mat_i);
    }
}

vk::Semaphore PrimitiveComponentWithMatrixColor::Draw(int swapchain_image_index, vk::Semaphore wait_sema)
{
    m_game.get_device().waitForFences(m_infos[swapchain_image_index].m_fence, VK_TRUE, -1);
    m_game.get_device().resetFences(m_infos[swapchain_image_index].m_fence);

    vk::PipelineStageFlags stage_flags = { vk::PipelineStageFlagBits::eBottomOfPipe };
    std::array command_buffers{ m_infos[swapchain_image_index].m_command_buffer };
    std::array queue_submits{ vk::SubmitInfo(wait_sema, stage_flags, command_buffers, m_infos[swapchain_image_index].m_sema) };
    m_game.get_queue().submit(queue_submits, m_infos[swapchain_image_index].m_fence);

    return m_infos[swapchain_image_index].m_sema;
}

void PrimitiveComponentWithMatrixColor::DestroyResources()
{
    m_game.get_device().freeCommandBuffers(m_game.get_command_pool(), m_command_buffers);
}

bool PrimitiveComponentWithMatrixColor::Intersect(const PrimitiveComponentWithMatrixColor& other)
{
    glm::vec4 own_center = m_world_matrix * glm::vec4(m_bounding_sphere.center, 1.0f);
    glm::vec4 other_center = other.m_world_matrix * glm::vec4(other.m_bounding_sphere.center, 1.0f);
    return does_intersect(BoundingSphere{ own_center, m_bounding_sphere.radius },
                          BoundingSphere{ other_center, other.m_bounding_sphere.radius });
}
