#pragma once

#include "export.h"

#include "IGameComponent.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

// This class is exported from the dll
class ENGINE_API Game {
public:
	Game();
	~Game();

//#ifdef _WIN32
	void Initialize(HINSTANCE hinstance, HWND hwnd, int width, int height);
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

    vk::RenderPass m_render_pass;
    vk::RenderPass m_game_component_render_pass;

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

    vk::RenderPass get_game_component_render_pass()
    {
        return m_game_component_render_pass;
    }

    vk::ShaderModule  loadSPIRVShader(std::string filename);
    uint32_t find_appropriate_memory_type(vk::MemoryRequirements& mem_req, const vk::PhysicalDeviceMemoryProperties& memory_props, vk::MemoryPropertyFlags memory_flags);
    // void AddGameComponent(std::unique_ptr<IGameComponent> component);
    void AddGameComponent(IGameComponent * component);
};
