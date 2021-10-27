#include "VulkanInitializer.h"

#include "TransformComponent.h"
#include "VulkanTransformComponent.h"
#include "MeshComponent.h"
#include "VulkanMeshComponent.h"
#include "CameraComponent.h"
#include "VulkanCameraComponent.h"
#include "UnlitMaterial.h"
#include "LitMaterial.h"
#include "Relation.h"

#include "util.h"

#include <stb_image.h>

namespace diffusion {

VulkanInitializer::VulkanInitializer(Game& game)
    : m_game(game)
{
    m_game.get_registry().on_construct<TransformComponent>().connect<&VulkanInitializer::add_vulkan_transform_component>(*this);
    m_game.get_registry().on_update<TransformComponent>().connect<&VulkanInitializer::transform_component_changed>(*this);
    m_game.get_registry().on_construct<SubMesh>().connect<&VulkanInitializer::add_vulkan_mesh_component>(*this);
    m_game.get_registry().on_construct<CameraComponent>().connect<&VulkanInitializer::add_vulkan_camera_component>(*this);
    m_game.get_registry().on_update<CameraComponent>().connect<&VulkanInitializer::camera_changed>(*this);
    m_game.get_registry().on_construct<UnlitMaterialComponent>().connect<&VulkanInitializer::search_for_unlit_material>(*this);
    m_game.get_registry().on_construct<LitMaterialComponent>().connect<&VulkanInitializer::search_for_lit_material>(*this);
}

void VulkanInitializer::add_vulkan_transform_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& transform = registry.get<TransformComponent>(parent_entity);

    const auto * mesh = registry.try_get<SubMesh>(parent_entity);
    const auto* vulkan_transform = registry.try_get<VulkanTransformComponent>(parent_entity);
    if (!mesh || vulkan_transform) {
        return;
    }

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };

    vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
    vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_game.get_descriptor_set_layouts()[1]))[0];

    std::vector matrixes{ calculate_global_world_matrix(registry, transform) };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);

    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(out2.m_buffer, {}, VK_WHOLE_SIZE) };
    std::array write_descriptors{ vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    registry.emplace<VulkanTransformComponent>(parent_entity, out2.m_buffer, out2.m_allocation, out2.m_mapped_memory, descriptor_pool, descriptor_set);
}

void VulkanInitializer::transform_component_changed(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& transform = registry.get<TransformComponent>(parent_entity);

    const auto * vulkan_transform = registry.try_get<VulkanTransformComponent>(parent_entity);
    if (vulkan_transform) {
        // Use patch instead of just editing memory to create on_update event
        registry.patch<VulkanTransformComponent>(parent_entity, [this, &transform, &registry](auto& vulkan_transform) {
            glm::mat4 world_matrix = calculate_global_world_matrix(registry, transform);
            std::memcpy(vulkan_transform.m_mapped_world_matrix_memory, &world_matrix, sizeof(glm::mat4));
        });
    }

    const auto * childs = registry.try_get<Childs>(parent_entity);
    if (childs) {
        for (auto& child : childs->m_childs) {
            registry.patch<TransformComponent>(child, [](auto& transform) {
            });
        }
    }
}

void VulkanInitializer::add_vulkan_mesh_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& mesh = registry.get<SubMesh>(parent_entity);

    const auto * transform = registry.try_get<TransformComponent>(parent_entity);
    if (transform) {
        add_vulkan_transform_component(registry, parent_entity);
    }


    auto vertex_memory = sync_create_host_invisible_buffer(m_game, mesh.m_verticies, vk::BufferUsageFlagBits::eVertexBuffer, 0);
    auto index_memory = sync_create_host_invisible_buffer(m_game, mesh.m_indexes, vk::BufferUsageFlagBits::eIndexBuffer, 0);

    registry.emplace<VulkanSubMesh>(parent_entity, vertex_memory.m_buffer, vertex_memory.m_allocation, index_memory.m_buffer, index_memory.m_allocation);
}

void VulkanInitializer::add_vulkan_camera_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& camera = registry.get<CameraComponent>(parent_entity);

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };

    vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
    vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_game.get_descriptor_set_layouts()[0]))[0];

    glm::mat4 view_projection_matrix = camera.m_projection_matrix * camera.m_camera_matrix;

    std::vector matrixes{ view_projection_matrix };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(out2.m_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_descriptors{ vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    registry.emplace<VulkanCameraComponent>(parent_entity, descriptor_pool, descriptor_set, view_projection_matrix, out2.m_buffer, out2.m_allocation, out2.m_mapped_memory);
}

void VulkanInitializer::camera_changed(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto camera_component = registry.get<CameraComponent>(parent_entity);
    camera_component.m_camera_matrix = glm::lookAt(
        camera_component.m_camera_position, // Позиция камеры в мировом пространстве
        camera_component.m_camera_target,   // Указывает куда вы смотрите в мировом пространстве
        camera_component.m_up_vector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );

    auto vulkan_camera_component = registry.get<VulkanCameraComponent>(parent_entity);
    vulkan_camera_component.m_view_projection_matrix = camera_component.m_projection_matrix * camera_component.m_camera_matrix;

    std::memcpy(vulkan_camera_component.m_world_view_projection_mapped_memory, &vulkan_camera_component.m_view_projection_matrix, sizeof(glm::mat4));
}

void VulkanInitializer::search_for_unlit_material(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& unlit_material_component = registry.get<UnlitMaterialComponent>(parent_entity);
    unlit_material_component.m_reference = ::entt::null;

    // search for entities with unlit materials
    registry.view<UnlitMaterial>().each([&registry, &parent_entity, &unlit_material_component](const UnlitMaterial & unlit_material) {
        // if there is a component with our textures
        if (unlit_material_component.m_albedo_path == unlit_material.m_albedo_path) {
            // set it's entity identifier to our component
            registry.patch<UnlitMaterialComponent>(parent_entity, [&unlit_material, &registry](UnlitMaterialComponent& unlit_material_component) {
                unlit_material_component.m_reference = ::entt::to_entity(registry, unlit_material);
            });
        }
    });

    // if there is no entity with our material
    if (!registry.valid(unlit_material_component.m_reference)) {
        // create new entity with our material
        auto new_material_entity = registry.create();

        std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };

        vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
        vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_game.get_descriptor_set_layouts()[2]))[0];

        auto command_buffer = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

        command_buffer.begin(vk::CommandBufferBeginInfo());
        ImageData albedo_image = prepare_image_for_copy(command_buffer, unlit_material_component.m_albedo_path);

        vk::ImageView albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, albedo_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        vk::Sampler albedo_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
        command_buffer.end();

        auto fence = m_game.get_device().createFence(vk::FenceCreateInfo());

        std::array command_buffers{ command_buffer };
        std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
        m_game.get_queue().submit(queue_submits, fence);

        m_game.get_device().waitForFences(fence, VK_TRUE, -1);
        m_game.get_device().destroyFence(fence);

        std::array descriptor_image_infos{ vk::DescriptorImageInfo(albedo_sampler, albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        m_game.get_device().updateDescriptorSets(vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}), {});

        registry.emplace<UnlitMaterial>(new_material_entity, albedo_image.m_image, albedo_image.m_memory, albedo_image_view, albedo_sampler, descriptor_pool, descriptor_set, unlit_material_component.m_albedo_path);

        // and assign our reference to created entity
        unlit_material_component.m_reference = new_material_entity;
    }

    registry.sort<UnlitMaterialComponent>([](const UnlitMaterialComponent& lhv, const UnlitMaterialComponent& rhv) {
        return lhv.m_reference < rhv.m_reference;
    });
}

void VulkanInitializer::search_for_lit_material(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& lit_material_component = registry.get<LitMaterialComponent>(parent_entity);
    lit_material_component.m_reference = ::entt::null;

    // search for entities with unlit materials
    registry.view<LitMaterial>().each([&registry, &parent_entity, &lit_material_component](const LitMaterial& lit_material) {
        // if there is a component with our textures
        if (lit_material_component.m_albedo_path == lit_material.m_albedo_path &&
            lit_material_component.m_normal_path == lit_material.m_normal_path) {
            // set it's entity identifier to our component
            registry.patch<LitMaterialComponent>(parent_entity, [&lit_material, &registry](LitMaterialComponent& lit_material_component) {
                lit_material_component.m_reference = ::entt::to_entity(registry, lit_material);
            });
        }
        });

    // if there is no entity with our material
    if (!registry.valid(lit_material_component.m_reference)) {
        // create new entity with our material
        auto new_material_entity = registry.create();

        std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };

        vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
        vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_game.get_descriptor_set_layouts()[2]))[0];

        auto command_buffer = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];

        command_buffer.begin(vk::CommandBufferBeginInfo());
        ImageData albedo_image = prepare_image_for_copy(command_buffer, lit_material_component.m_albedo_path);

        vk::ImageView albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, albedo_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        vk::Sampler albedo_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));


        ImageData normals_image = prepare_image_for_copy(command_buffer, lit_material_component.m_normal_path);

        vk::ImageView normal_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, normals_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), vk::ComponentMapping(vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eR), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        vk::Sampler normal_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
        command_buffer.end();

        auto fence = m_game.get_device().createFence(vk::FenceCreateInfo());

        std::array command_buffers{ command_buffer };
        std::array queue_submits{ vk::SubmitInfo({}, {}, command_buffers, {}) };
        m_game.get_queue().submit(queue_submits, fence);


        m_game.get_device().waitForFences(fence, VK_TRUE, -1);
        m_game.get_device().destroyFence(fence);


        std::array descriptor_image_infos{ vk::DescriptorImageInfo(albedo_sampler, albedo_image_view, vk::ImageLayout::eShaderReadOnlyOptimal),
                                           vk::DescriptorImageInfo(normal_sampler, normal_image_view, vk::ImageLayout::eShaderReadOnlyOptimal) };
        m_game.get_device().updateDescriptorSets(vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptor_image_infos, {}, {}), {});


        registry.emplace<LitMaterial>(new_material_entity,
            albedo_image.m_image, albedo_image.m_memory, albedo_image_view, albedo_sampler,
            normals_image.m_image, normals_image.m_memory, normal_image_view, normal_sampler,
            descriptor_pool, descriptor_set, lit_material_component.m_albedo_path, lit_material_component.m_normal_path);


        // and assign our reference to created entity
        lit_material_component.m_reference = new_material_entity;
    }

    registry.sort<LitMaterialComponent>([] (const LitMaterialComponent & lhv, const LitMaterialComponent & rhv) {
        return lhv.m_reference < rhv.m_reference;
    });
}

ImageData VulkanInitializer::prepare_image_for_copy(const vk::CommandBuffer& command_buffer, const std::filesystem::path& filepath)
{
    std::array<uint32_t, 1> queues{ 0 };

    int tex_width, tex_height, n;
    auto* data = stbi_load((std::filesystem::path("Materials") / filepath).string().c_str(), &tex_width, &tex_height, &n, 4);

    n = 4;

    VkDeviceSize imageSize = tex_width * tex_height * n;



    vk::Format texture_format = vk::Format::eB8G8R8A8Unorm;


    auto [image, image_memory] = m_game.get_allocator().createImage(
        vk::ImageCreateInfo({}, vk::ImageType::e2D, texture_format, vk::Extent3D(tex_width, tex_height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined /*ePreinitialized*/),
        vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));

    auto buffer_memory =
        m_game.get_allocator().createBuffer(vk::BufferCreateInfo({}, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, queues),
            vma::AllocationCreateInfo(vma::AllocationCreateInfo({}, vma::MemoryUsage::eCpuToGpu)));

    void* mapped_data = nullptr;
    m_game.get_allocator().mapMemory(buffer_memory.second, &mapped_data);


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

    //m_game.get_device().flushMappedMemoryRanges(vk::MappedMemoryRange(cpy_buffer_memory, {}, memory_buffer_req.size));
    m_game.get_allocator().unmapMemory(buffer_memory.second);

    std::array regions{ vk::BufferImageCopy({}, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {}, {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1}) };

    std::array start_barrier{ vk::ImageMemoryBarrier({}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };
    std::array finish_barrier{ vk::ImageMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)) };


    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, start_barrier);
    command_buffer.copyBufferToImage(buffer_memory.first, image, vk::ImageLayout::eTransferDstOptimal, regions);
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}/*vk::DependencyFlagBits::eViewLocal*/, {}, {}, finish_barrier);

    return { image, image_memory };
}


} // namespace diffusion {