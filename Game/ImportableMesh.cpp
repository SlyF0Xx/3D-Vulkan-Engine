#include "ImportableMesh.h"
#include "GameComponentMesh.h"
#include "util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

ImportableMesh::ImportableMesh(
    Game& game,
    GameComponentMesh& game_component,
    const std::vector<PrimitiveColoredVertex>& verticies,
    const std::vector<uint32_t>& indexes,
    const BoundingSphere& bounding_sphere)
    : m_verticies(verticies), m_indexes(indexes), m_game(game), m_game_component(game_component), m_bounding_sphere(bounding_sphere)
{
    m_game_component.join_to_game_component(*this);

    /*
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    */

    auto out = create_buffer(m_game, m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    m_vertex_buffer = out.m_buffer;
    m_vertex_memory = out.m_memory;

    auto out3 = create_buffer(m_game, m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);
    m_index_buffer = out3.m_buffer;
    m_index_memory = out3.m_memory;
}

void ImportableMesh::Draw(const vk::CommandBuffer& cmd_buffer)
{
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 1, m_game_component.get_descriptor_set(), { {} });

    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.bindIndexBuffer(m_index_buffer, {}, vk::IndexType::eUint32);
    cmd_buffer.drawIndexed(m_indexes.size(), m_indexes.size() / 3, 0, 0, 0);
}

glm::mat4 ImportableMesh::get_world_matrix() const
{
    return m_game_component.get_world_matrix();
}

bool ImportableMesh::Intersect(const ImportableMesh& other)
{
    glm::vec4 own_center = m_game_component.get_world_matrix() * glm::vec4(m_bounding_sphere.center, 1.0f);
    glm::vec4 other_center = other.get_world_matrix() * glm::vec4(other.m_bounding_sphere.center, 1.0f);
    return does_intersect(BoundingSphere{ own_center, m_bounding_sphere.radius },
        BoundingSphere{ other_center, other.m_bounding_sphere.radius });
}
