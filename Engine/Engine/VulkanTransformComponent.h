#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "TransformComponent.h"
#include "Engine.h"
#include "util.h"

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <cstring>

namespace diffusion {

class VulkanTransformComponent : public TransformComponent
{
public:
	VulkanTransformComponent(
		Game& game,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const std::vector<Tag>& tags);

	virtual void UpdateWorldMatrix(const glm::mat4& world_matrix) override;

	vk::DescriptorSet get_descriptor_set() const
	{
		return m_descriptor_set;
	}

	void Draw(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer);

	static Tag s_vulkan_transform_component_tag;

private:
	Game& m_game;
	vk::Buffer m_world_matrix_buffer;
	vk::DeviceMemory m_world_matrix_memory;

	vk::DescriptorPool m_descriptor_pool;
	vk::DescriptorSet m_descriptor_set;
};

} // namespace diffusion {