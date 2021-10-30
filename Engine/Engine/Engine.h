#pragma once

#include "IRender.h"
#include "Lights.h"
#include "VulkanInitializer.h"
#include "ComponentInitializer.h"
#include "glm_printer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.hpp>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <windows.h>

#include <optional>
#include <memory>
#include <unordered_set>
#include <unordered_map>

struct PrimitiveColoredVertex
{
    float x, y, z;
    //float colors[4];
    float tex_coords[2];
    float norm_coords[3];

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PrimitiveColoredVertex, x, y, z, tex_coords, norm_coords)
};

/*
#pragma pack(push, 1)
struct LightInfo
{
    glm::vec3 m_direction;
    uint32_t padding = 0;
};
#pragma pack(pop)
*/

// This class is exported from the dll
class Game {
public:
	Game();
	~Game();

    void InitializeSurface(vk::SurfaceKHR surface);
//#ifdef _WIN32

    void SecondInitialize();
//#endif // WIN_32

    void Update(int width, int height);


	void Exit();
	void Draw();

    int m_width;
    int m_height;
    bool m_initialized = false;

private:
    struct PerSwapchainImageData
    {
        vk::CommandBuffer m_command_buffer;

        // Constant per image value
        vk::Fence m_fence;
        vk::Semaphore m_sema;
    };
    vk::Semaphore m_sema;
    std::vector<vk::CommandBuffer> m_command_buffers;

    std::vector<PerSwapchainImageData> m_swapchain_data;

    entt::registry m_registry;
    diffusion::VulkanInitializer m_initializer;
    diffusion::ComponentInitializer m_component_initializer;

    vk::Instance m_instance;
    vk::PhysicalDevice m_phys_device;
    vk::Device m_device;
    uint32_t m_queue_family_index = -1;
    vk::Queue m_queue;
    vk::CommandPool m_command_pool;

    // material
    vk::Format m_color_format;// = vk::Format::eB8G8R8A8Unorm /* vk::Format::eR16G16B16A16Sfloat */;
    vk::Format m_depth_format;// = vk::Format::eD32SfloatS8Uint;
    vk::PhysicalDeviceMemoryProperties m_memory_props;

    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;

    size_t m_image_count = 2;
    vk::ComponentMapping m_texture_component_mapping;


    vk::DescriptorPool m_descriptor_pool;

    IRender* render;
    std::vector<vk::Image> images;

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    //vk::PipelineLayout m_layout;


    vk::DescriptorSet m_lights_descriptor_set;
    //std::vector<LightInfo> m_lights;
    vk::Buffer m_lights_buffer;
    vma::Allocation m_lights_memory;
    std::vector<std::byte> m_lights_memory_to_transfer;

    std::unique_ptr<Lights> m_shadpwed_lights;

    vk::Buffer m_lights_count_buffer;
    vma::Allocation m_lights_count_memory;
    std::vector<std::byte> m_lights_count_memory_to_transfer;

    vma::Allocator m_allocator;

    vk::PhysicalDevice select_physical_device();
    void select_graphics_queue_family();
    void InitializePipelineLayout();
    void InitializeColorFormats(const std::vector<vk::SurfaceFormatKHR>& formats);

public:
    const vk::Instance& get_instance()
    {
        return m_instance;
    }
    const vk::Device& get_device()
    {
        return m_device;
    }
    const vk::PhysicalDevice& get_physical_device()
    {
        return m_phys_device;
    }
    uint32_t get_queue_family_index()
    {
        return m_queue_family_index;
    }
    const vk::Queue& get_queue()
    {
        return m_queue;
    }
    const vk::CommandPool& get_command_pool()
    {
        return m_command_pool;
    }

    const vk::Format & get_color_format()
    {
        return m_color_format;
    }
    const vk::Format& get_depth_format()
    {
        return m_depth_format;
    }
    const vk::PhysicalDeviceMemoryProperties& get_memory_props()
    {
        return m_memory_props;
    }

    const vk::SurfaceKHR& get_surface()
    {
        return m_surface;
    }
    const vk::SwapchainKHR& get_swapchain()
    {
        return m_swapchain;
    }

    const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts()
    {
        return m_descriptor_set_layouts;
    }

    const vk::DescriptorSet& get_lights_descriptor_set()
    {
        return m_lights_descriptor_set;
    }

    /*
    const vk::PipelineLayout& get_layout()
    {
        return m_layout;
    }
    */

    Lights& get_shadpwed_lights()
    {
        return *m_shadpwed_lights;
    }

    vma::Allocator& get_allocator()
    {
        return m_allocator;
    }

    entt::registry& get_registry()
    {
        return m_registry;
    }

    vk::ComponentMapping get_texture_component_mapping()
    {
        return m_texture_component_mapping;
    }

    vk::ShaderModule  loadSPIRVShader(std::string filename);

    int add_light(const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector);
    //void update_light(int index, const LightInfo&);
};
