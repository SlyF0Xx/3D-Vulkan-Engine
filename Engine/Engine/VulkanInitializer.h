#pragma once

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

private:
    ImageData prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath);

    Game& m_game;
};

} // namespace diffusion {