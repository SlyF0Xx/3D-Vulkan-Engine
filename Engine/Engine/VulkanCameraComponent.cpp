#include "VulkanCameraComponent.h"

#include "util.h"

namespace diffusion {

VulkanCameraComponent::VulkanCameraComponent(Game& game, const std::vector<Tag>& tags, Entity* parent)
	: CameraComponent(game, concat_vectors({ s_vulkan_camera_component }, tags), parent)
{
	std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
	m_descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

	m_descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, m_game.get_descriptor_set_layouts()[0]))[0];

	m_view_projection_matrix = m_projection_matrix * m_camera_matrix;

	std::vector matrixes{ m_view_projection_matrix };
	auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
	m_world_view_projection_matrix_buffer = out2.m_buffer;
	m_world_view_projection_matrix_memory = out2.m_memory;
	m_world_view_projection_mapped_memory = out2.m_mapped_memory;

	std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(m_world_view_projection_matrix_buffer, {}, VK_WHOLE_SIZE) };

	std::array write_descriptors{ vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
	m_game.get_device().updateDescriptorSets(write_descriptors, {});
}

VulkanCameraComponent::~VulkanCameraComponent()
{
	m_game.get_device().unmapMemory(m_world_view_projection_matrix_memory);
}

void VulkanCameraComponent::recalculate_state()
{
	CameraComponent::recalculate_state();

	m_view_projection_matrix = m_projection_matrix * m_camera_matrix;

	std::vector matrixes{ m_view_projection_matrix };

	auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(m_world_view_projection_matrix_buffer);

	std::memcpy(m_world_view_projection_mapped_memory, matrixes.data(), sizeof(glm::mat4));
	m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(m_world_view_projection_matrix_memory, {}, memory_buffer_req.size));
}

void VulkanCameraComponent::Draw(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer)
{
	cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, m_descriptor_set, { {} });
}

} // namespace diffusion {