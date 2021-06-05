#pragma once

#include "export.h"

#include "IGameComponent.h"
#include "IMaterial.h"
#include "IMesh.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <memory>
#include <unordered_set>
#include <unordered_map>

struct PrimitiveColoredVertex
{
    float x, y, z;
    float color[4];
};

// This class is exported from the dll
class ENGINE_API Game {
public:
	Game();
	~Game();

    struct PerSwapchainImageData
    {
        vk::Image m_color_image;
        
        vk::Image m_deffered_depth_image;
        vk::DeviceMemory m_deffered_depth_memory;

        vk::Image m_depth_image;
        vk::DeviceMemory m_depth_memory;
        
        vk::Image m_albedo_image;
        vk::DeviceMemory m_albedo_memory;

        vk::ImageView m_color_image_view;
        vk::ImageView m_deffered_depth_image_view;
        vk::ImageView m_depth_image_view;
        vk::ImageView m_albedo_image_view;

        vk::Sampler m_albedo_sampler;

        vk::Framebuffer m_deffered_framebuffer;
        vk::Framebuffer m_composite_framebuffer;

        vk::CommandBuffer m_command_buffer;

        vk::Fence m_fence;
        vk::Semaphore m_sema;

        vk::DescriptorSet m_descriptor_set;
    };
//#ifdef _WIN32
	void Initialize(HINSTANCE hinstance, HWND hwnd, int width, int height);

    void SecondInitialize();
//#endif // WIN_32

    void Update(int width, int height);


	void Exit();
	void Draw();



    int m_width;
    int m_height;
    bool m_initialized = false;

private:
    vk::Instance m_instance;
    vk::Device m_device;
    vk::Queue m_queue;
    vk::CommandPool m_command_pool;

    // material
    vk::Format m_color_format = vk::Format::eB8G8R8A8Unorm /* vk::Format::eR16G16B16A16Sfloat */;
    vk::Format m_depth_format = vk::Format::eD32SfloatS8Uint;
    vk::PhysicalDeviceMemoryProperties m_memory_props;

    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;
    //std::vector<std::unique_ptr<IGameComponent>> m_game_components;
    std::vector<IGameComponent*> m_game_components;

    vk::Semaphore m_sema;

    std::vector<PerSwapchainImageData> m_swapchain_data;







    std::unordered_map<int, /*std::unique_ptr<*/IMaterial */*>*/> m_materials;
    std::unordered_map<MaterialType, std::unordered_set<int>> materials_by_type;
    std::unordered_map<int, std::unordered_set</*std::unique_ptr<*/IMesh */*>*/>> mesh_by_material;


    vk::RenderPass m_deffered_render_pass;
    vk::PipelineLayout m_layout;
    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_deffered_pipeline;

    std::array<vk::DescriptorSetLayout, 1> m_descriptor_set_layouts;


    vk::RenderPass m_composite_render_pass;
    vk::PipelineLayout m_composite_layout;
    vk::ShaderModule m_composite_vertex_shader;
    vk::ShaderModule m_composite_fragment_shader;
    vk::PipelineCache m_composite_cache;
    vk::Pipeline m_composite_pipeline;

    std::array<vk::DescriptorSetLayout, 2> m_composite_descriptor_set_layouts;
public:
    const vk::Instance& get_instance()
    {
        return m_instance;
    }
    const vk::Device& get_device()
    {
        return m_device;
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
    /*
    const std::vector<std::unique_ptr<IGameComponent>>& get_game_components()
    {
        return m_game_components;
    }
    */

    const vk::PipelineLayout& get_layout()
    {
        return m_layout;
    }

    const std::array<vk::DescriptorSetLayout, 1> & get_descriptor_set_layouts()
    {
        return m_descriptor_set_layouts;
    }

    const std::unordered_map<int, IMaterial*>& get_materials()
    {
        return m_materials;
    }

    const std::unordered_map<MaterialType, std::unordered_set<int>>& get_materials_by_type()
    {
        return materials_by_type;
    }

    const std::unordered_map<int, std::unordered_set<IMesh*>>& get_mesh_by_material()
    {
        return mesh_by_material;
    }

    const std::vector<IGameComponent*>& get_game_components()
    {
        return m_game_components;
    }
    const std::vector<PerSwapchainImageData>& get_swapchain_data()
    {
        return m_swapchain_data;
    }
    const vk::Semaphore& get_sema()
    {
        return m_sema;
    }

    vk::ShaderModule  loadSPIRVShader(std::string filename);
    uint32_t find_appropriate_memory_type(vk::MemoryRequirements& mem_req, const vk::PhysicalDeviceMemoryProperties& memory_props, vk::MemoryPropertyFlags memory_flags);
    // void AddGameComponent(std::unique_ptr<IGameComponent> component);
    void AddGameComponent(IGameComponent * component);

    void InitializeDefferedPipeline();
    void register_material(MaterialType material_type, /*std::unique_ptr<*/IMaterial */*>*/ material);
    void register_mesh(int material_id, /*std::unique_ptr<*/IMesh * /*>*/ mesh);

    void create_memory_for_image(const vk::Image& view, vk::DeviceMemory& memory);
};
