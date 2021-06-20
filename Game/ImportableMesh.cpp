#include "ImportableMesh.h"
#include "GameComponentMesh.h"
#include "util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <stb_image.h>

#include <fstream>

ImportableMesh::ImportableMesh(
    Game& game,
    GameComponentMesh& game_component,
    const std::vector<PrimitiveColoredVertex>& verticies,
    const std::vector<uint32_t>& indexes,
    const BoundingSphere& bounding_sphere)
    : m_verticies(verticies), m_indexes(indexes), m_game(game), m_game_component(game_component), m_bounding_sphere(bounding_sphere)
{
    m_game_component.join_to_game_component(*this);

    /*
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    */

    auto out = sync_create_host_invisible_buffer(m_game, m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    m_vertex_buffer = out.m_buffer;
    m_vertex_memory = out.m_memory;

    auto out3 = sync_create_host_invisible_buffer(m_game, m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);
    m_index_buffer = out3.m_buffer;
    m_index_memory = out3.m_memory;
}

void ImportableMesh::Draw(const vk::CommandBuffer& cmd_buffer)
{
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 1, m_game_component.get_descriptor_set(), { {} });

    cmd_buffer.bindVertexBuffers(0, m_vertex_buffer, { {0} });
    cmd_buffer.bindIndexBuffer(m_index_buffer, {}, vk::IndexType::eUint32);
    cmd_buffer.drawIndexed(m_indexes.size(), m_indexes.size() / 3, 0, 0, 0);
}

glm::mat4 ImportableMesh::get_world_matrix() const
{
    return m_game_component.get_world_matrix();
}

bool ImportableMesh::Intersect(const ImportableMesh& other)
{
    glm::vec4 own_center = m_game_component.get_world_matrix() * glm::vec4(m_bounding_sphere.center, 1.0f);
    glm::vec4 other_center = other.get_world_matrix() * glm::vec4(other.m_bounding_sphere.center, 1.0f);
    return does_intersect(BoundingSphere{ own_center, m_bounding_sphere.radius },
        BoundingSphere{ other_center, other.m_bounding_sphere.radius });
}




ImportableMaterial::ImportableMaterial(
    Game& game, const std::filesystem::path& texture_path)
    : m_game(game)
{
    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };
    m_descriptor_pool = game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));

    m_descriptor_set = game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_descriptor_pool, game.get_descriptor_set_layouts()[2]))[0];

    std::array<uint32_t, 1> queues{ 0 };

    int tex_width, tex_height, n;
    auto* data = stbi_load((std::filesystem::path("E:\\programming\\Graphics\\Game\\Game\\Materials") / texture_path).string().c_str(), &tex_width, &tex_height, &n, 4);

    /*
    std::ifstream file(std::filesystem::path("E:\\programming\\Graphics\\Game\\Game\\Materials") / texture_path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    */

    n = 4;

    VkDeviceSize imageSize = tex_width * tex_height * n;



    vk::Format texture_format = vk::Format::eB8G8R8A8Unorm;

    m_albedo_image = game.get_device().createImage(vk::ImageCreateInfo({}, vk::ImageType::e2D, texture_format, vk::Extent3D(tex_width, tex_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/));
    game.create_memory_for_image(m_albedo_image, m_albedo_memory);







    /* TODO: copy-paste from  */
    auto cpy_buffer = game.get_device().createBuffer(vk::BufferCreateInfo({}, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, queues));
    auto memory_buffer_req = game.get_device().getBufferMemoryRequirements(cpy_buffer);

    uint32_t buffer_index = game.find_appropriate_memory_type(memory_buffer_req, game.get_memory_props(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto cpy_buffer_memory = game.get_device().allocateMemory(vk::MemoryAllocateInfo(memory_buffer_req.size, buffer_index));

    void* mapped_data = nullptr;
    game.get_device().mapMemory(cpy_buffer_memory, {}, memory_buffer_req.size, {}, &mapped_data);

    auto staging_image_layout = game.get_device().getImageSubresourceLayout(m_albedo_image, vk::ImageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0));

    if (staging_image_layout.rowPitch == tex_width * n) {
        memcpy(mapped_data, data, (size_t)imageSize);
    }
    else {
        uint8_t* dataBytes = reinterpret_cast<uint8_t*>(mapped_data);

        for (int y = 0; y < tex_height; y++) {
            memcpy(&dataBytes[y * staging_image_layout.rowPitch], &data[y * tex_width * n], tex_width * n);
        }
    }

    game.get_device().bindBufferMemory(cpy_buffer, cpy_buffer_memory, {});
    game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(cpy_buffer_memory, {}, memory_buffer_req.size));
    game.get_device().unmapMemory(cpy_buffer_memory);









    auto command_buffer = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

    std::array regions{ vk::BufferImageCopy({}, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {}, {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1}) };

    std::array start_barrier{ vk::ImageMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, m_albedo_image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };
    std::array finish_barrier{ vk::ImageMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, m_albedo_image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };


    command_buffer.begin(vk::CommandBufferBeginInfo());
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands /*vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eFragmentShader*/, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, start_barrier);
    command_buffer.copyBufferToImage(cpy_buffer, m_albedo_image, vk::ImageLayout::eTransferDstOptimal, regions);
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, finish_barrier);
    command_buffer.end();



    auto fence = m_game.get_device().createFence(vk::FenceCreateInfo());

    std::array command_buffers{ command_buffer };
    std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
    m_game.get_queue().submit(queue_submits, fence);


    m_game.get_device().waitForFences(fence, VK_TRUE, -1);
    m_game.get_device().destroyFence(fence);




    m_albedo_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, m_albedo_image, vk::ImageViewType::e2D, game.get_color_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        
    m_albedo_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

    std::array descriptor_image_infos{ vk::DescriptorImageInfo(m_albedo_sampler, m_albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
    game.get_device().updateDescriptorSets(vk::WriteDescriptorSet(m_descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}), {});
}

void ImportableMaterial::UpdateMaterial(const vk::CommandBuffer& cmd_buffer)
{
    cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_game.get_layout(), 2, m_descriptor_set, { });
}

DefaultMaterial::DefaultMaterial(Game& game)
    : ImportableMaterial(game, "default.png")
{
}
