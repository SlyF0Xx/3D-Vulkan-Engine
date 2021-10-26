#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace diffusion::entt {

// generated in runtime - not persisted
struct LitMaterial
{
    vk::Image m_albedo_image;
    vma::Allocation m_albedo_memory;
    vk::ImageView m_albedo_image_view;
    vk::Sampler m_albedo_sampler;

    vk::Image m_normal_image;
    vma::Allocation m_normal_memory;
    vk::ImageView m_normal_image_view;
    vk::Sampler m_normal_sampler;

    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    std::filesystem::path m_albedo_path;
    std::filesystem::path m_normal_path;
};

struct LitMaterialComponent
{
    std::filesystem::path m_albedo_path;
    std::filesystem::path m_normal_path;
    ::entt::entity m_reference{ ::entt::null }; // will be assigned in runtime

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LitMaterialComponent, m_albedo_path, m_normal_path)
};

} // namespace diffusion::entt {