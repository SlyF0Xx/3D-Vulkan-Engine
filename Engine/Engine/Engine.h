#pragma once

#include "export.h"

#include "IGameComponent.h"
#include "IMaterial.h"
#include "IMesh.h"
#include "IRender.h"

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





    std::unordered_map<int, /*std::unique_ptr<*/IMaterial */*>*/> m_materials;
    std::unordered_map<MaterialType, std::unordered_set<int>> materials_by_type;
    std::unordered_map<int, std::unordered_set</*std::unique_ptr<*/IMesh */*>*/>> mesh_by_material;

    IRender* render;

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    vk::PipelineLayout m_layout;

    void InitializePipelineLayout();

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

    const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts()
    {
        return m_descriptor_set_layouts;
    }

    const vk::PipelineLayout& get_layout()
    {
        return m_layout;
    }

    vk::ShaderModule  loadSPIRVShader(std::string filename);
    uint32_t find_appropriate_memory_type(vk::MemoryRequirements& mem_req, const vk::PhysicalDeviceMemoryProperties& memory_props, vk::MemoryPropertyFlags memory_flags);

    void register_material(MaterialType material_type, /*std::unique_ptr<*/IMaterial */*>*/ material);
    void register_mesh(int material_id, /*std::unique_ptr<*/IMesh * /*>*/ mesh);

    void create_memory_for_image(const vk::Image& view, vk::DeviceMemory& memory);
};
