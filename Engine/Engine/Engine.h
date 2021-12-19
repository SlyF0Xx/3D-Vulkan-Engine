#pragma once

#include "IRender.h"
#include "VulkanInitializer.h"
#include "ComponentInitializer.h"
#include "BaseComponents/ScriptComponent.h"
#include "glm_printer.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <taskflow/taskflow.hpp> 

#include <windows.h>

#include <optional>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "IMenuRenderer.h"

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

struct PresentationEngine
{
    // Per Image Data
    struct PerSwapchainImageData
    {
        vk::Image m_color_image;

        vk::Image m_depth_image;
        vma::Allocation m_depth_memory;

        vk::ImageView m_color_image_view;
        vk::ImageView m_depth_image_view;

        vk::CommandBuffer m_command_buffer;

        // Constant per image value
        vk::Fence m_fence;
    };

    struct SemaData
    {
        vk::Semaphore m_image_acquired_sema;
        vk::Semaphore m_render_complete_sema;
    };

    int                 m_width;
    int                 m_height;

    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;

    vk::Format m_color_format;
    vk::Format m_depth_format;

    vk::PresentModeKHR m_presentation_mode; // TODO: is it really needed?
    vk::ImageLayout m_final_layout;

    uint32_t            FrameIndex = 0;         // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    uint32_t            m_image_count = 2;      // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
    uint32_t            SemaphoreIndex = 0;     // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)

    std::vector<PerSwapchainImageData> m_swapchain_data;
    std::vector<SemaData> m_sema_data;

    bool use_frame_index_to_render = true;;
};

// This class is exported from the dll
class Game {
public:
	Game();
	~Game();

    void InitializePresentationEngine(const PresentationEngine & presentation_engine);
//#ifdef _WIN32

    void SecondInitialize();
//#endif // WIN_32

    //void Update(int width, int height);
    void Update();

	void Exit();
	void Draw();
    void DrawRestruct();

    bool m_initialized = false;

    int                 m_shadow_width = 1024;
    int                 m_shadow_height = 1024;
    bool m_stopped = true;

    std::chrono::steady_clock::time_point m_script_time_point = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point m_phys_time_point = std::chrono::steady_clock::now();

    bool m_paused = true;
    std::mutex m_render_mutex;

private:
    // Vulkan Common Data
    vk::Instance m_instance;
    vk::PhysicalDevice m_phys_device;
    vk::Device m_device;
    uint32_t m_queue_family_index = -1;
    vk::Queue m_queue;
    vk::CommandPool m_command_pool;

    vma::Allocator m_allocator;

    PresentationEngine m_presentation_engine;

    vk::DescriptorPool m_descriptor_pool; // TODO: make more common

    // GPU based properties
    vk::ComponentMapping m_texture_component_mapping;


    // Entity Component System Data
    entt::registry m_registry;
    diffusion::VulkanInitializer m_initializer;
    diffusion::ComponentInitializer m_component_initializer;



    std::unique_ptr<IRender> render;
    std::unique_ptr<IMenuRenderer> m_menu_renderer;

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    lua_State* m_lua_state;

    tf::Executor m_executor;
    tf::Taskflow m_taskflow;
    std::vector<tf::Task> m_tasks;

    vk::PhysicalDevice select_physical_device();
    void select_graphics_queue_family();
    void InitializePipelineLayout();
    void InitializeColorFormats(const std::vector<vk::SurfaceFormatKHR>& formats, PresentationEngine& presentation_engine);

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

    vma::Allocator& get_allocator()
    {
        return m_allocator;
    }
    PresentationEngine get_presentation_engine()
    {
        return m_presentation_engine;
    }

    const vk::SurfaceKHR& get_surface()
    {
        return m_presentation_engine.m_surface;
    }
    const vk::SwapchainKHR& get_swapchain()
    {
        return m_presentation_engine.m_swapchain;
    }

    const vk::Format & get_color_format()
    {
        return m_presentation_engine.m_color_format;
    }
    const vk::Format& get_depth_format()
    {
        return m_presentation_engine.m_depth_format;
    }
    const diffusion::ImageData get_texture(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath) {
        return m_initializer.prepare_image_for_copy(command_buffer, filepath);
    }
    vk::ComponentMapping get_texture_component_mapping()
    {
        return m_texture_component_mapping;
    }


    entt::registry& get_registry()
    {
        return m_registry;
    }
    void register_menu_renderer(std::unique_ptr<IMenuRenderer> menu_renderer)
    {
        m_menu_renderer = std::move(menu_renderer);
    }


    const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts()
    {
        return m_descriptor_set_layouts;
    }
    vk::DescriptorPool get_descriptor_pool()
    {
        return m_descriptor_pool;
    }

    lua_State* get_lua_state()
    {
        return m_lua_state;
    }

    tf::Executor & get_executor()
    {
        return m_executor;
    }

    tf::Taskflow & get_taskflow()
    {
        return m_taskflow;
    }

    std::vector<tf::Task>& get_tasks()
    {
        return m_tasks;
    }

    diffusion::ComponentInitializer & get_component_initializer()
    { return m_component_initializer; }

    vk::ShaderModule  loadSPIRVShader(std::string filename);

    //void update_light(int index, const LightInfo&);

    PresentationEngine create_default_presentation_engine(HINSTANCE hinstance, HWND hwnd);
    float get_depth(size_t x, size_t y);

    void load_scene(const std::filesystem::path & path = "sample_scene.json");
    void save_scene(const std::filesystem::path & path = "sample_scene.json");

    void render_tick();
    void run();
    void pause();
    void resume();
    void stop();
};
