#pragma once

#include "ImportableMesh.h"

#include <glm/glm.hpp>

#include <vector>

class GameComponentMesh
{

private:
	Game& m_game;
	GameComponentMesh* m_parent = nullptr;
	std::vector<ImportableMesh> m_meshes;
	
	glm::mat4 m_world_matrix;

	vk::Buffer m_world_matrix_buffer;
	vk::DeviceMemory m_world_matrix_memory;

	vk::DescriptorPool m_descriptor_pool;
	vk::DescriptorSet m_descriptor_set;

public:
	GameComponentMesh(
		Game& game,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);

	void InitializeWorldMatrix(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);

	void set_parrent(GameComponentMesh* parent);

	void join_to_game_component(const ImportableMesh & mesh);

	void UpdateWorldMatrix(const glm::mat4& world_matrix);

	glm::mat4 get_world_matrix() const
	{
		if (!m_parent) {
			return m_world_matrix;
		}
		else {
			return m_parent->get_world_matrix() * m_world_matrix;
		}
	}

	vk::DescriptorSet get_descriptor_set() const
	{
		return m_descriptor_set;
	}
};

