#pragma once

#include "MeshComponent.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.hpp>

namespace diffusion {

// generated in runtime - not persisted
struct VulkanSubMesh
{
    vk::Buffer m_vertex_buffer;
    vma::Allocation m_vertex_memory;
    vk::Buffer m_index_buffer;
    vma::Allocation m_index_memory;
};

} // namespace diffusion {