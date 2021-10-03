#pragma once

#include "System.h"
#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"

namespace diffusion {

class VulkanRenderSystem :
    public System
{
public:
    void components_callback(const std::vector<std::reference_wrapper<Component>>& components) override {
        for (auto& component : components) {
            auto it = std::find(
                component.get().get_tags().begin(),
                component.get().get_tags().end(),
                VulkanTransformComponent::s_vulkan_transform_component_tag);
            if (it != component.get().get_tags().end()) {
                auto comp = dynamic_cast<VulkanTransformComponent&>(component.get());
                //comp.Draw();
            }
            else {
                auto comp = dynamic_cast<VulkanMeshComponent&>(component.get());
                //comp.Draw();
            }
        }
    }
};

} // namespace diffusion {