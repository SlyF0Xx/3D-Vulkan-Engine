#pragma once

#include "System.h"
#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"
#include "Entity.h"

namespace diffusion {

class VulkanRenderSystem :
    public System
{
public:
#if 0
    void on_update() {
        std::array colors{ vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{ 0.3f, 0.3f, 0.3f, 1.0f })),
                           vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0))
        };

        m_command_buffers = m_game.get_device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_game.get_command_pool(), vk::CommandBufferLevel::ePrimary, 2));
        for (int i = 0; i < m_swapchain_data.size(); ++i) {
            m_swapchain_data[i].m_command_buffer = m_command_buffers[i];

            m_swapchain_data[i].m_command_buffer.begin(vk::CommandBufferBeginInfo());

            for (int j = 0; j < m_game.get_shadpwed_lights().get_shadowed_light().size(); ++j) {
                m_game.get_shadpwed_lights().get_shadowed_light()[j].InitCommandBuffer(i, m_swapchain_data[i].m_command_buffer);
            }

            m_swapchain_data[i].m_command_buffer.beginRenderPass(vk::RenderPassBeginInfo(m_render_pass, m_swapchain_data[i].m_framebuffer, vk::Rect2D({}, vk::Extent2D(m_game.m_width, m_game.m_height)), colors), vk::SubpassContents::eInline);
            m_swapchain_data[i].m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

            m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, m_game.get_descriptor_set(), { {} }); /*view_proj_binding*/
            m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 3, m_game.get_lights_descriptor_set(), {});
            m_swapchain_data[i].m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 4, m_swapchain_data[i].m_shadows_descriptor_set, {});

            vk::Viewport viewport(0, 0, m_game.m_width, m_game.m_height, 0.0f, 1.0f);
            m_swapchain_data[i].m_command_buffer.setViewport(0, viewport);
            vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(m_game.m_width, m_game.m_height));
            m_swapchain_data[i].m_command_buffer.setScissor(0, scissor);

            for (auto& [mat_type, materials] : m_game.get_materials_by_type()) {
                for (auto& material : materials) {
                    m_game.get_materials().find(material)->second->UpdateMaterial(m_layout, m_swapchain_data[i].m_command_buffer);




                    for (auto& component : components) {
                        Entity * parent = component.get().get_parrent();
                        for (auto& inner_component : parent->get_components()) {
                            auto it = std::find(
                                inner_component.get().get_tags().begin(),
                                inner_component.get().get_tags().end(),
                                VulkanTransformComponent::s_vulkan_transform_component_tag);
                            if (it != inner_component.get().get_tags().end()) {
                                auto comp = dynamic_cast<VulkanTransformComponent&>(inner_component.get());
                                comp.Draw(m_layout, m_swapchain_data[i].m_command_buffer);
                            }
                        }

                        auto comp = dynamic_cast<VulkanMeshComponent&>(component.get());
                        comp.Draw(m_swapchain_data[i].m_command_buffer);
                    }




                }
            }

            m_swapchain_data[i].m_command_buffer.endRenderPass();
            m_swapchain_data[i].m_command_buffer.end();
        }
    }

private:
    struct PerSwapchainImageData
    {
        // Variable per image value
        vk::Image m_color_image;

        vk::Image m_depth_image;
        vk::DeviceMemory m_depth_memory;

        vk::ImageView m_color_image_view;
        vk::ImageView m_depth_image_view;

        vk::Framebuffer m_framebuffer;

        vk::CommandBuffer m_command_buffer;

        vk::DescriptorSet m_shadows_descriptor_set;

        // Constant per image value
        vk::Fence m_fence;
        vk::Semaphore m_sema;
    };

    std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts;
    vk::PipelineLayout m_layout;

    vk::Semaphore m_sema;
    std::vector<vk::CommandBuffer> m_command_buffers;

    std::vector<PerSwapchainImageData> m_swapchain_data;

    vk::RenderPass m_render_pass;
    vk::ShaderModule m_vertex_shader;
    vk::ShaderModule m_fragment_shader;
    vk::PipelineCache m_cache;
    vk::Pipeline m_pipeline;



    Game& m_game;

#endif
};

} // namespace diffusion {