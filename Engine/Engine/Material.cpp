#include "Material.h"

#include <stb_image.h>

ImageData UnlitMaterial::prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath)
{
    std::array<uint32_t, 1> queues{ 0 };

    int tex_width, tex_height, n;
    auto* data = stbi_load((std::filesystem::path("Materials") / filepath).string().c_str(), &tex_width, &tex_height, &n, 4);

    n = 4;

    VkDeviceSize imageSize = tex_width * tex_height * n;



    vk::Format texture_format = vk::Format::eB8G8R8A8Unorm;

    auto image = m_game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, texture_format, vk::Extent3D(tex_width, tex_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
    vk::DeviceMemory memory;
    m_game.create_memory_for_image(image, memory);



    /* TODO: copy-paste from  */
    auto cpy_buffer = m_game.get_device().createBuffer(vk::BufferCreateInfo({}, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, queues));
    auto memory_buffer_req = m_game.get_device().getBufferMemoryRequirements(cpy_buffer);

    uint32_t buffer_index = m_game.find_appropriate_memory_type(memory_buffer_req, m_game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto cpy_buffer_memory = m_game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

    void* mapped_data = nullptr;
    m_game.get_device().mapMemory(cpy_buffer_memory, {}, memory_buffer_req.size, {}, &mapped_data);

    auto staging_image_layout = m_game.get_device().getImageSubresourceLayout(image, vk::ImageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0));

    if (staging_image_layout.rowPitch == tex_width * n) {
        memcpy(mapped_data, data, (size_t)imageSize);
    }
    else {
        uint8_t* dataBytes = reinterpret_cast<uint8_t*>(mapped_data);

        for (int y = 0; y < tex_height; y++) {
            memcpy(&dataBytes[y * staging_image_layout.rowPitch], &data[y * tex_width * n], tex_width * n);
        }
    }

    m_game.get_device().bindBufferMemory(cpy_buffer, cpy_buffer_memory, {});
    m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(cpy_buffer_memory, {}, memory_buffer_req.size));
    m_game.get_device().unmapMemory(cpy_buffer_memory);

    std::array regions{ vk::BufferImageCopy({}, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {}, {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1}) };

    std::array start_barrier{ vk::ImageMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };
    std::array finish_barrier{ vk::ImageMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };


    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, start_barrier);
    command_buffer.copyBufferToImage(cpy_buffer, image, vk::ImageLayout::eTransferDstOptimal, regions);
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, finish_barrier);

    return { image, memory };
}

UnlitMaterial::UnlitMaterial(Game& game)
    : m_game(game)
{
}

UnlitMaterial::UnlitMaterial(Game& game, const std::filesystem::path& texture_path)
    : m_game(game)
{
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, game.get_descriptor_set_layouts()[2]))[0];

    auto command_buffer = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

    command_buffer.begin(vk::CommandBufferBeginInfo());
    ImageData albedo_image = prepare_image_for_copy(command_buffer, texture_path);
    m_albedo_image = albedo_image.m_image;
    m_albedo_memory = albedo_image.m_memory;

    m_albedo_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_albedo_image, vk::ImageViewType::e2D, game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
    m_albedo_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
    command_buffer.end();

    auto fence = m_game.get_device().createFence(vk::FenceCreateInfo());

    std::array command_buffers{ command_buffer };
    std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
    m_game.get_queue().submit(queue_submits, fence);


    m_game.get_device().waitForFences(fence, VK_TRUE, -1);
    m_game.get_device().destroyFence(fence);

    std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_albedo_sampler, m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
    game.get_device().updateDescriptorSets(vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}), {});
}

void UnlitMaterial::UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer)
{
    int unlit = 1;
    cmd_buffer.pushConstants(layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
    cmd_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 0);
    cmd_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 0);
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 2, m_descriptor_set, { });
}

ImportableMaterial::ImportableMaterial(
    Game& game, const std::filesystem::path& texture_path, const std::filesystem::path& normal_path)
    : UnlitMaterial(game)
{
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, game.get_descriptor_set_layouts()[2]))[0];

    auto command_buffer = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

    command_buffer.begin(vk::CommandBufferBeginInfo());
    ImageData albedo_image = prepare_image_for_copy(command_buffer, texture_path);
    m_albedo_image = albedo_image.m_image;
    m_albedo_memory = albedo_image.m_memory;

    m_albedo_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_albedo_image, vk::ImageViewType::e2D, game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
    m_albedo_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));



    ImageData normals_image = prepare_image_for_copy(command_buffer, normal_path);
    m_normal_image = normals_image.m_image;
    m_normal_memory = normals_image.m_memory;

    m_normal_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_normal_image, vk::ImageViewType::e2D, game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
    m_normal_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
    command_buffer.end();

    auto fence = m_game.get_device().createFence(vk::FenceCreateInfo());

    std::array command_buffers{ command_buffer };
    std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
    m_game.get_queue().submit(queue_submits, fence);


    m_game.get_device().waitForFences(fence, VK_TRUE, -1);
    m_game.get_device().destroyFence(fence);



    std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_albedo_sampler, m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal),
                                       vk::DescriptorImageInfo(m_normal_sampler, m_normal_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
    game.get_device().updateDescriptorSets(vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}), {});
}

void ImportableMaterial::UpdateMaterial(const vk::PipelineLayout& layout, const vk::CommandBuffer& cmd_buffer)
{
    int unlit = 0;
    cmd_buffer.pushConstants(layout, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex /*because of layout*/, 0, sizeof(int), &unlit);
    cmd_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, 1);
    cmd_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, 1);
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 2, m_descriptor_set, { });
}

DefaultMaterial::DefaultMaterial(Game& game)
    : UnlitMaterial(game, "default.png")
{
}