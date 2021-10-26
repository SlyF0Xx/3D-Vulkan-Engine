#pragma once
#include "CameraComponent.h"

#include <vk_mem_alloc.hpp>

namespace diffusion {

// generated in runtime - not persisted
struct VulkanCameraComponent
{
    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;
    glm::mat4 m_view_projection_matrix;
    vk::Buffer m_world_view_projection_matrix_buffer;
    vma::Allocation m_world_view_projection_matrix_memory;
    std::byte* m_world_view_projection_mapped_memory;
};

} // namespace diffusion {