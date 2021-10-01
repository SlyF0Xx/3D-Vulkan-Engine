#include "VulkanTransformComponent.h"

namespace diffusion {

VulkanTransformComponent::VulkanTransformComponent(
	Game& game,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
	: TransformComponent(position, rotation, scale), m_game(game)
{
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

void VulkanTransformComponent::UpdateWorldMatrix(const glm::mat4& world_matrix)
{
	TransformComponent::UpdateWorldMatrix(world_matrix);

	std::vector matrixes{ get_world_matrix() };

	auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_matrix_buffer);

	void* mapped_data = nullptr;
	m_game.get_device().mapMemory(m_world_matrix_memory, {}, memory_buffer_req.size, {}, &mapped_data);
	std::memcpy(mapped_data, matrixes.data(), sizeof(glm::mat4));
	m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_matrix_memory, {}, memory_buffer_req.size));
	m_game.get_device().unmapMemory(m_world_matrix_memory);
}

} // namespace diffusion {