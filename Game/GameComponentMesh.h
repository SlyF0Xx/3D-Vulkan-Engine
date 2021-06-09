#pragma once

#include "ImportableMesh.h"

#include <glm/glm.hpp>

#include <vector>

class GameComponentMesh
{
private:
	Game& m_game;
	GameComponentMesh* m_parent;
	std::vector<ImportableMesh> m_meshes;
	
	glm::mat4 m_world_matrix;

	vk::Buffer m_world_matrix_buffer;
	vk::DeviceMemory m_world_matrix_memory;

	vk::DescriptorPool m_descriptor_pool;
	vk::DescriptorSet m_descriptor_set;

public:
	GameComponentMesh(
		Game& game,
		GameComponentMesh * parent,
		const std::vector<ImportableMesh>& meshes,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);

	void InitializeWorldMatrix(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);

	void UpdateWorldMatrix(const glm::mat4& world_matrix);

	glm::mat4 get_world_matrix()
	{
		if (!m_parent) {
			return m_world_matrix;
		}
		else {
			return m_parent->get_world_matrix() * m_world_matrix;
		}
	}

	vk::DescriptorSet get_descriptor_set()
	{
		return m_descriptor_set;
	}
};

