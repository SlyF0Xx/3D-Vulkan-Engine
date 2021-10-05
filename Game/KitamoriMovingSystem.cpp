#include "KitamoriMovingSystem.h"
#include "Entity.h"
#include "TransformComponent.h"

#include "VulkanTransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

void KitamoriMovingSystem::update_position(glm::vec3 direction)
{
    for (auto& linked_component : m_linked_components) {
        for (auto& inner_component : linked_component->get_parrent()->get_components()) {
            auto it = std::find(
                inner_component.get().get_tags().begin(),
                inner_component.get().get_tags().end(),
                //diffusion::VulkanTransformComponent::s_vulkan_transform_component_tag);
                diffusion::TransformComponent::s_transform_component_tag);
            if (it != inner_component.get().get_tags().end()) {
                auto & comp = dynamic_cast<diffusion::TransformComponent&>(inner_component.get());
                comp.UpdateWorldMatrix(glm::translate(comp.get_world_matrix(), direction));
            }
        }
    }

    update_components();
}

} // namespace diffusion {