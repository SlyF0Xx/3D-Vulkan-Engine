#include "VulkanInitializer.h"

#include "BaseComponents/TransformComponent.h"
#include "BaseComponents/VulkanComponents/VulkanTransformComponent.h"
#include "BaseComponents/MeshComponent.h"
#include "BaseComponents/VulkanComponents/VulkanMeshComponent.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/VulkanComponents/VulkanCameraComponent.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/DirectionalLightComponent.h"
#include "BaseComponents/VulkanComponents/VulkanDirectionalLightComponent.h"
#include "BaseComponents/PointLightComponent.h"
#include "BaseComponents/DebugComponent.h"

#include "util.h"

#include <stb_image.h>
#include <glm/gtx/matrix_decompose.hpp>

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

    m_game.get_registry().on_construct<DirectionalLightComponent>().connect<&VulkanInitializer::add_directional_light>(*this);
    m_game.get_registry().on_construct<PointLightComponent>().connect<&VulkanInitializer::add_point_light>(*this);

    //m_game.get_registry().on_construct<Relation>().connect<&VulkanInitializer::change_transform_component>(*this);
    m_game.get_registry().on_construct<Relation>().connect<&VulkanInitializer::change_transform_component>(*this);

    m_game.get_registry().on_destroy<VulkanDirectionalLights>().connect<&VulkanInitializer::destroy_vulkan_lights_component>(*this);

    //m_game.get_registry().on_destroy<PointLightComponent>().connect<&VulkanInitializer::destroy_point_light>(*this);
}

void VulkanInitializer::destroy_vulkan_lights_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto lights = registry.ctx<VulkanDirectionalLights>();
    m_game.get_device().freeDescriptorSets(m_game.get_descriptor_pool(), { lights.m_lights_descriptor_set });
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

struct PointCamera
{
    glm::vec3 m_position;
    float padding = 0;
    glm::mat4 m_projection_matrix;
};

void VulkanInitializer::change_transform_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto* transform = registry.try_get<TransformComponent>(parent_entity);
    if (transform) {
        transform_component_changed(registry, parent_entity);
    }
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

    if (registry.try_get<CameraComponent>(parent_entity)) {
        camera_changed(registry, parent_entity);
    }

    if (registry.try_get<VulkanDirectionalLightComponent>(parent_entity)) {
        if (registry.try_get<VulkanPointLightCamera>(parent_entity)) {
            registry.patch<diffusion::VulkanPointLightCamera>(parent_entity, [&registry, &parent_entity, &transform, this](const diffusion::VulkanPointLightCamera& vulkan_camera) {
                auto& camera = registry.get<PointLightComponent>(parent_entity);
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(transform.m_world_matrix, scale, rotation, translation, skew, perspective);

                std::vector<PointCamera> matrixes{ PointCamera{ translation, 0, camera.m_projection_matrix } };
                auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
                std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(out2.m_buffer, {}, VK_WHOLE_SIZE) };

                std::array write_descriptors{ vk::WriteDescriptorSet(vulkan_camera.m_descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptor_buffer_infos, {}) };
                m_game.get_device().updateDescriptorSets(write_descriptors, {});
                });
        }

        auto& lights = registry.ctx<VulkanDirectionalLights>();
        update_lights_buffer(registry, &lights);
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

void VulkanInitializer::calculate_view_matrix(::entt::registry& registry, CameraComponent& camera, TransformComponent& transform)
{
    auto camera_view = calculate_camera_view(registry, camera, transform);

    camera.m_camera_matrix = glm::lookAt(
        camera_view.position, // Позиция камеры в мировом пространстве
        camera_view.target,   // Указывает куда вы смотрите в мировом пространстве
        camera_view.up        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );
    camera.m_view_projection_matrix = camera.m_projection_matrix * camera.m_camera_matrix;
}

void VulkanInitializer::add_vulkan_camera_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& camera = registry.get<CameraComponent>(parent_entity);
    auto& transform = registry.get<TransformComponent>(parent_entity);
    calculate_view_matrix(registry, camera, transform);

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1) };

    vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
    vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, m_game.get_descriptor_set_layouts()[0]))[0];

    std::vector matrixes{ camera.m_view_projection_matrix };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(out2.m_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_descriptors{ vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    registry.emplace<VulkanCameraComponent>(parent_entity, descriptor_pool, descriptor_set, out2.m_buffer, out2.m_allocation, out2.m_mapped_memory);
}

void VulkanInitializer::camera_changed(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& camera_component = registry.get<CameraComponent>(parent_entity);
    auto& transform = registry.get<TransformComponent>(parent_entity);
    calculate_view_matrix(registry, camera_component, transform);

    auto vulkan_camera_component = registry.get<VulkanCameraComponent>(parent_entity);
    std::memcpy(vulkan_camera_component.m_world_view_projection_mapped_memory, &camera_component.m_view_projection_matrix, sizeof(glm::mat4));
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
        ImageData albedo_image = prepare_image_for_copy(command_buffer, get_materials_path(unlit_material_component.m_albedo_path));

        vk::ImageView albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, albedo_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), m_game.get_texture_component_mapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
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
        ImageData albedo_image = prepare_image_for_copy(command_buffer, get_materials_path(lit_material_component.m_albedo_path));

        vk::ImageView albedo_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, albedo_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), m_game.get_texture_component_mapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        vk::Sampler albedo_sampler = m_game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));


        ImageData normals_image = prepare_image_for_copy(command_buffer, get_materials_path(lit_material_component.m_normal_path));

        vk::ImageView normal_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, normals_image.m_image, vk::ImageViewType::e2D, m_game.get_color_format(), m_game.get_texture_component_mapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
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
    std::cout << "LOADING Texture - " << filepath << std::endl;

    std::array<uint32_t, 1> queues{ 0 };

    int tex_width, tex_height, n;
    auto* data = stbi_load(filepath.string().c_str(), &tex_width, &tex_height, &n, 4);

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

std::filesystem::path VulkanInitializer::get_materials_path(const std::filesystem::path& filepath) {
    return std::filesystem::path("Materials") / filepath;
}

vk::RenderPass VulkanInitializer::initialize_render_pass()
{
    std::array attachment_descriptions{ vk::AttachmentDescription{{}, m_game.get_depth_format(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilReadOnlyOptimal} };

    vk::AttachmentReference depth_attachment(0, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    /*
    std::array subpass_description{ vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics,
                                                           {},
                                                           {},
                                                           {},
                                                           &depth_attachment) };
    */
    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics);
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depth_attachment;

    std::array dependencies{ vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                               vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                               vk::AccessFlagBits::eShaderRead,
                                               vk::AccessFlagBits::eDepthStencilAttachmentWrite/*,
                                               vk::DependencyFlagBits::eByRegion*/),
                         vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL,
                                               vk::PipelineStageFlagBits::eLateFragmentTests,
                                               vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                               vk::AccessFlagBits::eShaderRead/*,
                                               vk::DependencyFlagBits::eByRegion*/)
    };

    return m_game.get_device().createRenderPass(vk::RenderPassCreateInfo({}, attachment_descriptions, subpass, dependencies));
}

namespace {

auto initialize_depth_data(Game& game, size_t new_light_entities_size)
{
    std::array<uint32_t, 1> queues{ 0 };
    auto depth_allocation = game.get_allocator().createImage(
        vk::ImageCreateInfo({}, vk::ImageType::e2D, game.get_depth_format(), vk::Extent3D(game.m_shadow_width, game.m_shadow_height, 1), 1, new_light_entities_size, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined),
        vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));

    auto depth_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, depth_allocation.first, vk::ImageViewType::e2DArray, game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, new_light_entities_size)));
    auto depth_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

    return std::make_tuple(depth_allocation, depth_image_view, depth_sampler);
}

auto initialize_point_depth_data(Game& game, size_t new_light_entities_size)
{
    std::array<uint32_t, 1> queues{ 0 };
    auto depth_allocation = game.get_allocator().createImage(
        vk::ImageCreateInfo(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, game.get_depth_format(), vk::Extent3D(game.m_shadow_width, game.m_shadow_height, 1), 1, new_light_entities_size * 6, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queues, vk::ImageLayout::eUndefined),
        vma::AllocationCreateInfo({}, vma::MemoryUsage::eGpuOnly));

    auto depth_image_view = game.get_device().createImageView(vk::ImageViewCreateInfo({}, depth_allocation.first, vk::ImageViewType::eCubeArray, game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, new_light_entities_size * 6)));
    auto depth_sampler = game.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));

    return std::make_tuple(depth_allocation, depth_image_view, depth_sampler);
}

VulkanDirectionalLights::PipelineInfo initialize_render_pipeline(Game& game, const vk::RenderPass & render_pass)
{
    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
        game.get_descriptor_set_layouts()[0],
        game.get_descriptor_set_layouts()[1]
    };

    vk::PipelineLayout layout = game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, descriptor_set_layouts, {}));

    vk::ShaderModule vertex_shader = game.loadSPIRVShader("ShadowMap.vert.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertex_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, game.m_shadow_width, game.m_shadow_height, 0.0f, 1.0f) }; /* TODO: shadow map resolution */
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(game.m_shadow_width, game.m_shadow_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, {});

    vk::PipelineCache cache = game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = game.get_device().createGraphicsPipeline(cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, layout, render_pass));
    vk::Pipeline pipeline = pipeline_result.value;

    return VulkanDirectionalLights::PipelineInfo{ std::move(descriptor_set_layouts), layout, vertex_shader, cache, pipeline };
}

VulkanDirectionalLights::PipelineInfo initialize_point_render_pipeline(Game& game, const vk::RenderPass& render_pass)
{
    std::array view_proj_binding{
       vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eGeometry, nullptr) /*view_proj*/
    };

    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
        game.get_device().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, view_proj_binding)),
        game.get_descriptor_set_layouts()[1]
    };

    vk::PipelineLayout layout = game.get_device().createPipelineLayout(vk::PipelineLayoutCreateInfo({}, descriptor_set_layouts, {}));

    vk::ShaderModule vertex_shader = game.loadSPIRVShader("PoinLightShadowMap.vert.spv");
    vk::ShaderModule geom_shader = game.loadSPIRVShader("PointLight.geom.spv");

    std::array stages{ vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertex_shader, "main"),
                       vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eGeometry, geom_shader, "main")
    };

    std::array vertex_input_bindings{ vk::VertexInputBindingDescription(0, sizeof(PrimitiveColoredVertex), vk::VertexInputRate::eVertex) };
    std::array vertex_input_attributes{ vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat),
                                        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)),
                                        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)) };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, vertex_input_bindings, vertex_input_attributes);
    vk::PipelineInputAssemblyStateCreateInfo input_assemply({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    std::array viewports{ vk::Viewport(0, 0, game.m_shadow_width, game.m_shadow_height, 0.0f, 1.0f) }; /* TODO: shadow map resolution */
    std::array scissors{ vk::Rect2D(vk::Offset2D(), vk::Extent2D(game.m_shadow_width, game.m_shadow_height)) };
    vk::PipelineViewportStateCreateInfo viewport_state({}, viewports, scissors);

    vk::PipelineRasterizationStateCreateInfo rasterization_info({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineDepthStencilStateCreateInfo depth_stensil_info({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1000.0f  /*Depth test*/);

    vk::PipelineMultisampleStateCreateInfo multisample/*({}, vk::SampleCountFlagBits::e1)*/;
    vk::PipelineColorBlendStateCreateInfo blend_state({}, VK_FALSE, vk::LogicOp::eClear, {});

    vk::PipelineCache cache = game.get_device().createPipelineCache(vk::PipelineCacheCreateInfo());

    std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic({}, dynamic_states);

    auto pipeline_result = game.get_device().createGraphicsPipeline(cache, vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_info, &input_assemply, {}, &viewport_state, &rasterization_info, &multisample, &depth_stensil_info, &blend_state, &dynamic, layout, render_pass));
    vk::Pipeline pipeline = pipeline_result.value;

    return VulkanDirectionalLights::PipelineInfo{ std::move(descriptor_set_layouts), layout, vertex_shader, cache, pipeline };
}

struct Lights {
    float light_buffer_size;
    float point_light_buffer_size;
    float padding[2];
    LightShaderInfo lights[10];
    PointLightInfo point_lights[10];
};

} // unnamed namespace

void VulkanInitializer::init_vulkan_lights(::entt::registry& registry, VulkanDirectionalLights*& lights_ptr, std::vector<vk::Image> & images, bool is_directional)
{
    vk::RenderPass render_pass = initialize_render_pass();

    std::vector<VulkanDirectionalLights::PerSwapchainImageData> swapchain_data(m_game.get_presentation_engine().m_image_count);
    for (int i = 0; i < swapchain_data.size(); ++i) {
        if (is_directional)
        {   
            auto depth_data = initialize_depth_data(m_game, 1);
            swapchain_data[i].m_directional_light_info.m_depth_image = std::get<0>(depth_data).first;
            swapchain_data[i].m_directional_light_info.m_depth_memory = std::get<0>(depth_data).second;
            swapchain_data[i].m_directional_light_info.m_depth_image_view = std::get<1>(depth_data);
            swapchain_data[i].m_directional_light_info.m_depth_sampler = std::get<2>(depth_data);
            images.push_back(std::get<0>(depth_data).first);
        }
        else
        {
            auto depth_data = initialize_point_depth_data(m_game, 1);
            swapchain_data[i].m_point_light_info.m_depth_image = std::get<0>(depth_data).first;
            swapchain_data[i].m_point_light_info.m_depth_memory = std::get<0>(depth_data).second;
            swapchain_data[i].m_point_light_info.m_depth_image_view = std::get<1>(depth_data);
            swapchain_data[i].m_point_light_info.m_depth_sampler = std::get<2>(depth_data);
            images.push_back(std::get<0>(depth_data).first);
        }
    }

    auto pipeline = initialize_render_pipeline(m_game, render_pass);
    auto point_pipeline = initialize_point_render_pipeline(m_game, render_pass);


    auto sets = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_game.get_descriptor_pool(), m_game.get_descriptor_set_layouts()[3]));

    auto m_lights_buffer_memory = sync_create_empty_host_invisible_buffer(m_game, sizeof(Lights), vk::BufferUsageFlagBits::eUniformBuffer, 0);
    std::array lights_buffer_infos{ vk::DescriptorBufferInfo(m_lights_buffer_memory.m_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_lights_descriptors{ vk::WriteDescriptorSet(sets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, lights_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_lights_descriptors, {});

    lights_ptr = &registry.set<VulkanDirectionalLights>(
        std::move(swapchain_data),
        render_pass,
        pipeline,
        point_pipeline,
        std::vector<entt::entity>{},
        std::vector<entt::entity>{},
        sets[0],
        m_lights_buffer_memory.m_buffer,
        m_lights_buffer_memory.m_allocation);
}

void VulkanInitializer::recreate_framebuffer(::entt::registry& registry, std::vector<vk::Image> images, VulkanDirectionalLights& lights, int i, int layer_count)
{
    auto& vulkan_light = registry.get<VulkanDirectionalLightComponent>(lights.m_directional_light_entities[i]);

    for (int i = 0; i < images.size(); ++i) {
        m_game.get_device().destroyImageView(vulkan_light.m_swapchain_data[i].m_depth_image_view);
        m_game.get_device().destroyFramebuffer(vulkan_light.m_swapchain_data[i].m_framebuffer);
    }

    for (int j = 0; j < vulkan_light.m_swapchain_data.size(); ++j) {
        vulkan_light.m_swapchain_data[j].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, images[j], vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, i, layer_count)));

        std::array deffered_views{ vulkan_light.m_swapchain_data[j].m_depth_image_view };
        vulkan_light.m_swapchain_data[j].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, lights.m_render_pass, deffered_views, m_game.m_shadow_width, m_game.m_shadow_height, layer_count));
    }
}

void VulkanInitializer::recreate_point_framebuffer(::entt::registry& registry, std::vector<vk::Image> images, VulkanDirectionalLights& lights, int i, int layer_count)
{
    auto& vulkan_light = registry.get<VulkanDirectionalLightComponent>(lights.m_point_light_entities[i]);

    for (int i = 0; i < images.size(); ++i) {
        m_game.get_device().destroyImageView(vulkan_light.m_swapchain_data[i].m_depth_image_view);
        m_game.get_device().destroyFramebuffer(vulkan_light.m_swapchain_data[i].m_framebuffer);
    }

    for (int j = 0; j < vulkan_light.m_swapchain_data.size(); ++j) {
        vulkan_light.m_swapchain_data[j].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, images[j], vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, i, layer_count)));

        std::array deffered_views{ vulkan_light.m_swapchain_data[j].m_depth_image_view };
        vulkan_light.m_swapchain_data[j].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, lights.m_render_pass, deffered_views, m_game.m_shadow_width, m_game.m_shadow_height, layer_count));
    }
}

void VulkanInitializer::update_lights_buffer(::entt::registry& registry, VulkanDirectionalLights* lights_ptr)
{
    Lights lights;
    lights.light_buffer_size = lights_ptr->m_directional_light_entities.size();
    lights.point_light_buffer_size = lights_ptr->m_point_light_entities.size();
    for (int i = 0; i < lights_ptr->m_directional_light_entities.size(); ++i) {
        auto& light_camera = registry.get<CameraComponent>(lights_ptr->m_directional_light_entities[i]);
        auto& transform = registry.get<TransformComponent>(lights_ptr->m_directional_light_entities[i]);
        auto camera_view = calculate_camera_view(registry, light_camera, transform);

        lights.lights[i].m_direction = camera_view.target - camera_view.position;
        lights.lights[i].m_ViewProjection = light_camera.m_view_projection_matrix;
    }
    for (int i = 0; i < lights_ptr->m_point_light_entities.size(); ++i) {
        auto& light_camera = registry.get<TransformComponent>(lights_ptr->m_point_light_entities[i]);
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(light_camera.m_world_matrix, scale, rotation, translation, skew, perspective);
        lights.point_lights[i].m_position = translation;
    }

    update_buffer(m_game, sizeof(lights), &lights, lights_ptr->m_lights_buffer, vk::BufferUsageFlagBits::eUniformBuffer, 0);
}
    
void VulkanInitializer::add_directional_light(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto * lights_ptr = registry.try_ctx<VulkanDirectionalLights>();
    std::vector<vk::Image> images;
    if (!lights_ptr) {
        init_vulkan_lights(registry, lights_ptr, images, true);
    }
    else {
        VulkanDirectionalLights& lights = *lights_ptr;

        std::array<uint32_t, 1> queues{ 0 };
        //TODO: registry.patch(); on VulkanDirectionalLights
        for (int i = 0; i < lights.m_swapchain_data.size(); ++i) {
            if (lights.m_directional_light_entities.size() != 0)
            {
                m_game.get_device().destroySampler(lights.m_swapchain_data[i].m_directional_light_info.m_depth_sampler);
                m_game.get_device().destroyImageView(lights.m_swapchain_data[i].m_directional_light_info.m_depth_image_view);

                m_game.get_allocator().destroyImage(lights.m_swapchain_data[i].m_directional_light_info.m_depth_image, lights.m_swapchain_data[i].m_directional_light_info.m_depth_memory);   
            }

            auto depth_data = initialize_depth_data(m_game, lights.m_directional_light_entities.size() + 1);
            lights.m_swapchain_data[i].m_directional_light_info.m_depth_image = std::get<0>(depth_data).first;
            lights.m_swapchain_data[i].m_directional_light_info.m_depth_memory = std::get<0>(depth_data).second;
            lights.m_swapchain_data[i].m_directional_light_info.m_depth_image_view = std::get<1>(depth_data);
            lights.m_swapchain_data[i].m_directional_light_info.m_depth_sampler = std::get<2>(depth_data);

            images.push_back(std::get<0>(depth_data).first);
        }

        for (int i = 0; i < lights.m_directional_light_entities.size(); ++i) {
            recreate_framebuffer(registry, images, lights, i, 1);
        }
    }

    std::vector<VulkanDirectionalLightComponent::PerSwapchainImageData> swapchain_data(images.size());
    std::array<uint32_t, 1> queues{ 0 };

    for (int i = 0; i < swapchain_data.size(); ++i) {
        swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, images[i], vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, lights_ptr->m_directional_light_entities.size(), 1)));

        std::array deffered_views{ swapchain_data[i].m_depth_image_view };
        swapchain_data[i].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, lights_ptr->m_render_pass, deffered_views, m_game.m_shadow_width, m_game.m_shadow_height, 1));
    }

    registry.emplace<VulkanDirectionalLightComponent>(parent_entity, std::move(swapchain_data));
    lights_ptr->m_directional_light_entities.push_back(parent_entity);

    m_game.Update();

    update_lights_buffer(registry, lights_ptr);
}

void VulkanInitializer::add_point_light(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto* lights_ptr = registry.try_ctx<VulkanDirectionalLights>();
    std::vector<vk::Image> images;
    if (!lights_ptr) {
        init_vulkan_lights(registry, lights_ptr, images, false);
    }
    else {
        VulkanDirectionalLights& lights = *lights_ptr;

        std::array<uint32_t, 1> queues{ 0 };
        //TODO: registry.patch(); on VulkanDirectionalLights
        for (int i = 0; i < lights.m_swapchain_data.size(); ++i) {
            if (lights.m_point_light_entities.size() != 0)
            {
                m_game.get_device().destroySampler(lights.m_swapchain_data[i].m_point_light_info.m_depth_sampler);
                m_game.get_device().destroyImageView(lights.m_swapchain_data[i].m_point_light_info.m_depth_image_view);

                m_game.get_allocator().destroyImage(lights.m_swapchain_data[i].m_point_light_info.m_depth_image, lights.m_swapchain_data[i].m_point_light_info.m_depth_memory);   
            }

            auto depth_data = initialize_point_depth_data(m_game, lights.m_point_light_entities.size() + 1);
            lights.m_swapchain_data[i].m_point_light_info.m_depth_image = std::get<0>(depth_data).first;
            lights.m_swapchain_data[i].m_point_light_info.m_depth_memory = std::get<0>(depth_data).second;
            lights.m_swapchain_data[i].m_point_light_info.m_depth_image_view = std::get<1>(depth_data);
            lights.m_swapchain_data[i].m_point_light_info.m_depth_sampler = std::get<2>(depth_data);

            images.push_back(std::get<0>(depth_data).first);
        }

        for (int i = 0; i < lights.m_point_light_entities.size(); ++i) {
            recreate_point_framebuffer(registry, images, lights, i, 6);
        }
    }

    std::vector<VulkanDirectionalLightComponent::PerSwapchainImageData> swapchain_data(images.size());
    std::array<uint32_t, 1> queues{ 0 };

    for (int i = 0; i < swapchain_data.size(); ++i) {
        swapchain_data[i].m_depth_image_view = m_game.get_device().createImageView(vk::ImageViewCreateInfo({}, images[i], vk::ImageViewType::e2D, m_game.get_depth_format(), vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, lights_ptr->m_point_light_entities.size(), 6)));

        std::array deffered_views{ swapchain_data[i].m_depth_image_view };
        swapchain_data[i].m_framebuffer = m_game.get_device().createFramebuffer(vk::FramebufferCreateInfo({}, lights_ptr->m_render_pass, deffered_views, m_game.m_shadow_width, m_game.m_shadow_height, 6));
    }


    auto& camera = registry.get<PointLightComponent>(parent_entity);
    auto& transform = registry.get<TransformComponent>(parent_entity);
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transform.m_world_matrix, scale, rotation, translation, skew, perspective);

    std::array pool_size{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1) };

    vk::DescriptorPool descriptor_pool = m_game.get_device().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, pool_size));
    vk::DescriptorSet descriptor_set = m_game.get_device().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptor_pool, lights_ptr->m_point_light_pipeline.m_descriptor_set_layouts[0]))[0];

    std::vector matrixes{ PointCamera{ translation, 0, camera.m_projection_matrix } };
    auto out2 = create_buffer(m_game, matrixes, vk::BufferUsageFlagBits::eUniformBuffer, 0, false);
    std::array descriptor_buffer_infos{ vk::DescriptorBufferInfo(out2.m_buffer, {}, VK_WHOLE_SIZE) };

    std::array write_descriptors{ vk::WriteDescriptorSet(descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptor_buffer_infos, {}) };
    m_game.get_device().updateDescriptorSets(write_descriptors, {});

    registry.emplace<VulkanPointLightCamera>(parent_entity, descriptor_pool, descriptor_set, out2.m_buffer, out2.m_allocation, out2.m_mapped_memory);
    registry.emplace<VulkanDirectionalLightComponent>(parent_entity, std::move(swapchain_data));
    lights_ptr->m_point_light_entities.push_back(parent_entity);

    //m_game.Update();

    update_lights_buffer(registry, lights_ptr);
}


void VulkanInitializer::init_light_command_buffer(Game& game, diffusion::VulkanDirectionalLights& light, int i, const vk::CommandBuffer& command_buffer, std::vector<entt::entity>::value_type& entity)
{
    std::array colors{ vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0)) };

    diffusion::VulkanDirectionalLightComponent& vulkan_light = game.get_registry().get<diffusion::VulkanDirectionalLightComponent>(entity);

    command_buffer.beginRenderPass(vk::RenderPassBeginInfo(light.m_render_pass, vulkan_light.m_swapchain_data[i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(game.m_shadow_width, game.m_shadow_height)), colors), vk::SubpassContents::eInline);




    diffusion::VulkanPointLightCamera* point_camera = game.get_registry().try_get<diffusion::VulkanPointLightCamera>(entity);
    if (point_camera) {
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, light.m_point_light_pipeline.m_layout, 0, point_camera->m_descriptor_set, {});
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, light.m_point_light_pipeline.m_pipeline);
    }
    else {
        diffusion::VulkanCameraComponent& vulkan_camera = game.get_registry().get<diffusion::VulkanCameraComponent>(entity);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, light.m_directional_light_pipeline.m_layout, 0, vulkan_camera.m_descriptor_set, { {} });
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, light.m_directional_light_pipeline.m_pipeline);
    }





    vk::Viewport viewport(0, 0, game.m_shadow_width, game.m_shadow_height, 0.0f, 1.0f);
    command_buffer.setViewport(0, viewport);
    vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(game.m_shadow_width, game.m_shadow_height));
    command_buffer.setScissor(0, scissor);

    auto view = game.get_registry().view<
        const diffusion::VulkanTransformComponent,
        const diffusion::VulkanSubMesh,
        const diffusion::SubMesh>(entt::exclude<diffusion::debug_tag>);

    view.each([&light, &command_buffer, &point_camera](
        const diffusion::VulkanTransformComponent& transform,
        const diffusion::VulkanSubMesh& vulkan_mesh,
        const diffusion::SubMesh& mesh) {

            if (point_camera) {
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, light.m_point_light_pipeline.m_layout, 1, transform.m_descriptor_set, { {} });
            }
            else {
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, light.m_directional_light_pipeline.m_layout, 1, transform.m_descriptor_set, { {} });
            }             
                
                
            command_buffer.bindVertexBuffers(0, vulkan_mesh.m_vertex_buffer, { {0} });
            command_buffer.bindIndexBuffer(vulkan_mesh.m_index_buffer, {}, vk::IndexType::eUint32);
            command_buffer.drawIndexed(mesh.m_indexes.size(), 1, 0, 0, 0);
        });

    command_buffer.endRenderPass();
}

void VulkanInitializer::init_command_buffer(Game& game, diffusion::VulkanDirectionalLights & light, int i, const vk::CommandBuffer& command_buffer)
{
    for (auto& entity : light.m_directional_light_entities) {
        init_light_command_buffer(game, light, i, command_buffer, entity);
    }
    for (auto& entity : light.m_point_light_entities) {
        init_light_command_buffer(game, light, i, command_buffer, entity);
    }
}

} // namespace diffusion {