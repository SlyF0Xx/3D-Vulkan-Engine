#pragma once

#include "BaseComponents/VulkanComponents/VulkanDirectionalLightComponent.h"

#include <entt/entt.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <filesystem>

class Game;

namespace diffusion {

struct ImageData {
    vk::Image m_image;
    vma::Allocation m_memory;
};

class VulkanInitializer
{
public:
    VulkanInitializer(Game& game);

    void add_vulkan_mesh_component(::entt::registry& registry, ::entt::entity parent_entity);
    void add_vulkan_transform_component(::entt::registry& registry, ::entt::entity parent_entity);
    void transform_component_changed(::entt::registry& registry, ::entt::entity parent_entity);
    void add_vulkan_camera_component(::entt::registry& registry, ::entt::entity parent_entity);
    void camera_changed(::entt::registry& registry, ::entt::entity parent_entity);
    void search_for_unlit_material(::entt::registry& registry, ::entt::entity parent_entity);
    void search_for_lit_material(::entt::registry& registry, ::entt::entity parent_entity);
    vk::RenderPass initialize_render_pass();
    void init_vulkan_lights(::entt::registry& registry, VulkanDirectionalLights*& lights_ptr, std::vector<vk::Image> & images, bool is_directional);
    void recreate_framebuffer(::entt::registry& registry, std::vector<vk::Image> images,
                              VulkanDirectionalLights& lights,
                              int i, int layer_count);
    void add_directional_light(::entt::registry& registry, ::entt::entity parent_entity);
    void update_lights_buffer(::entt::registry& registry, VulkanDirectionalLights* lights_ptr);
    void add_point_light(::entt::registry& registry, ::entt::entity parent_entity);
    static void init_light_command_buffer(Game& game, diffusion::VulkanDirectionalLights& light, int i,
                                          const vk::CommandBuffer& command_buffer,
                                          std::vector<entt::entity>::value_type& entity);
    static void init_command_buffer(Game& game, diffusion::VulkanDirectionalLights& light, int i, const vk::CommandBuffer& command_buffer);

    ImageData prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath);

private:
    static std::filesystem::path get_materials_path(const std::filesystem::path& filepath);

    Game& m_game;
};

} // namespace diffusion {