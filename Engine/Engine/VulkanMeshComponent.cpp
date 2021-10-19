#include "VulkanMeshComponent.h"
#include "util.h"

namespace diffusion {

VulkanMeshComponent::VulkanMeshComponent(
    Game& game,
    const std::vector<PrimitiveColoredVertex>& verticies,
    const std::vector<uint32_t>& indexes,
    const std::vector<Tag>& tags,
    Entity* parent)
    : MeshComponent(game, verticies, indexes, concat_vectors({ s_vulkan_mesh_component_tag }, tags), parent)
{
    auto out = sync_create_host_invisible_buffer(m_game, m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    m_vertex_buffer = out;

    auto out3 = sync_create_host_invisible_buffer(m_game, m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);
    m_index_buffer = out3;
}

void VulkanMeshComponent::Draw(const vk::CommandBuffer& cmd_buffer)
{
    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.bindIndexBuffer(m_index_buffer, {}, vk::IndexType::eUint32);
    cmd_buffer.drawIndexed(m_indexes.size(), m_indexes.size() / 3, 0, 0, 0);
}

} // namespace diffusion {