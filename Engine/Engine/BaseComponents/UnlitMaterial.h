#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace diffusion {

// generated in runtime - not persisted
struct UnlitMaterial
{
    vk::Image m_albedo_image;
    vma::Allocation m_albedo_memory;
    vk::ImageView m_albedo_image_view;
    vk::Sampler m_albedo_sampler;

    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    std::filesystem::path m_albedo_path;
};

struct UnlitMaterialComponent
{
    std::filesystem::path m_albedo_path;
    ::entt::entity m_reference{::entt::null}; // will be assigned in runtime

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UnlitMaterialComponent, m_albedo_path)
};

} // namespace diffusion {