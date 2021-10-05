#pragma once

#include "Engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <filesystem>

struct ImageData {
    vk::Image m_image;
    vk::DeviceMemory m_memory;
};

class UnlitMaterial : public IMaterial
{
protected:
    vk::Image m_albedo_image;
    vk::DeviceMemory m_albedo_memory;
    vk::ImageView m_albedo_image_view;
    vk::Sampler m_albedo_sampler;

    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorSet m_descriptor_set;

    Game& m_game;

    ImageData prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath);

public:
    UnlitMaterial(Game& game);
    UnlitMaterial(Game& game, const std::filesystem::path& texture_path);

    void UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer) override;
};

class ImportableMaterial : public UnlitMaterial
{
private:
    vk::Image m_normal_image;
    vk::DeviceMemory m_normal_memory;
    vk::ImageView m_normal_image_view;
    vk::Sampler m_normal_sampler;

public:
    ImportableMaterial(
        Game& game, const std::filesystem::path& texture_path, const std::filesystem::path& normal_path);
    void UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer) override;
};

class DefaultMaterial : public UnlitMaterial
{
public:
    DefaultMaterial(Game& game);
};
