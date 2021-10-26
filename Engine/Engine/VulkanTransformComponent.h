#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "TransformComponent.h"
#include "Engine.h"
#include "util.h"

#include <glm/glm.hpp>

#include <vk_mem_alloc.hpp>

#include <entt/entt.hpp>

#include <array>
#include <vector>
#include <cstring>

namespace diffusion {

// generated in runtime - not persisted
struct VulkanTransformComponent
{
	vk::Buffer m_world_matrix_buffer;
	vma::Allocation m_world_matrix_memory;
	std::byte* m_mapped_world_matrix_memory;

	vk::DescriptorPool m_descriptor_pool;
	vk::DescriptorSet m_descriptor_set;
};

} // namespace diffusion {