#pragma once

#include "MeshComponent.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

namespace diffusion {

class VulkanMeshComponent :
    public MeshComponent
{
public:
    VulkanMeshComponent(
        Game& game,
        const std::vector<PrimitiveColoredVertex>& verticies,
        const std::vector<uint32_t>& indexes,
        const std::vector<Tag>& tags,
        Entity* parent);

    void Draw(const vk::CommandBuffer& cmd_buffer);

    inline static Tag s_vulkan_mesh_component_tag;

private:
    vk::Buffer m_vertex_buffer;
    vk::DeviceMemory m_vertex_memory;

    vk::Buffer m_index_buffer;
    vk::DeviceMemory m_index_memory;
};

} // namespace diffusion {