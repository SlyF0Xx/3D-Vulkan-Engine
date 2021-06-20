#include "GameComponentMesh.h"

#include "util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

GameComponentMesh::GameComponentMesh(
	Game& game,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
	: m_game(game)
{
	InitializeWorldMatrix(position, rotation, scale);

	std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
	m_descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

	m_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, m_game.get_descriptor_set_layouts()[1]))[0];

	std::vector matrixes{ get_world_matrix() };
	auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0);
	m_world_matrix_buffer = out2.m_buffer;
	m_world_matrix_memory = out2.m_memory;

	std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_matrix_buffer, {}, VK_WHOLE_SIZE) };
	std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
	m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

void GameComponentMesh::InitializeWorldMatrix(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
{
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1), position);

	glm::mat4 rotation_matrix(1);

	glm::vec3 RotationX(1.0, 0, 0);
	rotation_matrix = glm::rotate(glm::mat4(1), rotation[0], RotationX);

	glm::vec3 RotationY(0, 1.0, 0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[1], RotationY);

	glm::vec3 RotationZ(0, 0, 1.0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[2], RotationZ);

	glm::mat4 scale_matrix = glm::scale(scale);

	m_world_matrix = translation_matrix * rotation_matrix * scale_matrix;
}

void GameComponentMesh::set_parrent(GameComponentMesh* parent)
{
	m_parent = parent;

	std::vector matrixes{ get_world_matrix() };

	auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);

	void* mapped_data = nullptr;
	m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
	std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
	m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
	m_game.get_device().unmapMemory(m_world_matrix_memory);
}

void GameComponentMesh::join_to_game_component(const ImportableMesh& mesh)
{
	m_meshes.push_back(mesh);
}

void GameComponentMesh::UpdateWorldMatrix(const glm::mat4& world_matrix)
{
	m_world_matrix = world_matrix;

	// TODO: update childrens descriptor sets

	/*
	std::vector matrixes{ m_world_matrix };

	auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);

	void* mapped_data = nullptr;
	m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
	std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
	m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
	m_game.get_device().unmapMemory(m_world_matrix_memory);
	*/
}
