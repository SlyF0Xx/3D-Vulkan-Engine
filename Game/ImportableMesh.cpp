#include "ImportableMesh.h"
#include "util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

ImportableMesh::ImportableMesh(
    Game& game,
    const std::filesystem::path& path,
    const BoundingSphere& bounding_sphere,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale,
    const glm::mat4& CameraMatrix,
    const glm::mat4& ProjectionMatrix)
    : m_game(game), m_bounding_sphere(bounding_sphere)
{
    /*
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    */
    InitializeWorldMatrix(position, rotation, scale);

    m_view_matrix = CameraMatrix;
    m_projection_matrix = ProjectionMatrix;
    m_world_view_projection_matrix = m_projection_matrix * m_view_matrix * m_world_matrix;




    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, m_game.get_descriptor_set_layouts()))[0];

    std::vector matrixes{ m_world_view_projection_matrix };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_world_matrix_buffer = out2.m_buffer;
    m_world_matrix_memory = out2.m_memory;




    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});




    auto out = create_buffer(m_game, m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    m_vertex_buffer = out.m_buffer;
    m_vertex_memory = out.m_memory;

    auto out3 = create_buffer(m_game, m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);
    m_index_buffer = out3.m_buffer;
    m_index_memory = out3.m_memory;


}

void ImportableMesh::InitializeWorldMatrix(
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale)
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
}

void ImportableMesh::Draw(const vk::CommandBuffer& cmd_buffer)
{
    //cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, m_descriptor_set, {});
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 0, m_descriptor_set, { {} });

    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.bindIndexBuffer(m_index_buffer, {}, vk::IndexType::eUint32);
    cmd_buffer.drawIndexed(m_indexes.size(), m_indexes.size() / 3, 0, 0, 0);
}

glm::mat4 ImportableMesh::get_world_matrix()
{
    return m_world_matrix;
}

void ImportableMesh::UpdateWorldMatrix(const glm::mat4& world_matrix)
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
}

void ImportableMesh::UpdateViewMatrix(const glm::mat4& view_matrix)
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
}

void ImportableMesh::SetWVPMatrix(const glm::mat4& world_view_projection_matrix)
{
    m_world_view_projection_matrix = world_view_projection_matrix;

    std::vector matrixes{ m_world_view_projection_matrix };

    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);

    void* mapped_data = nullptr;
    m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
    std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
    m_game.get_device().unmapMemory(m_world_matrix_memory);
}

bool ImportableMesh::Intersect(const ImportableMesh& other)
{
    glm::vec4 own_center = m_world_matrix * glm::vec4(m_bounding_sphere.center, 1.0f);
    glm::vec4 other_center = other.m_world_matrix * glm::vec4(other.m_bounding_sphere.center, 1.0f);
    return does_intersect(BoundingSphere{ own_center, m_bounding_sphere.radius },
        BoundingSphere{ other_center, other.m_bounding_sphere.radius });
}
