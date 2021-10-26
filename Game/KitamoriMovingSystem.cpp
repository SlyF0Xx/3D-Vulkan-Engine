#include "KitamoriMovingSystem.h"
#include "Entity.h"
#include "TransformComponent.h"

#include "VulkanTransformComponent.h"
#include "KitamoriSystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

namespace diffusion {

void KitamoriMovingSystem::update_position(glm::vec3 direction)
{
    auto potential_linked_components = m_registry.view<const entt::KitamoriLinkedTag>();
    potential_linked_components.each([this, &direction](const entt::KitamoriLinkedTag& tag) {
        m_registry.patch<entt::TransformComponent>(::entt::to_entity(m_registry, tag), [&direction](entt::TransformComponent& transform) {
            transform.m_world_matrix = glm::translate(transform.m_world_matrix, direction);
        });
    });

    /*
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
    */

    update_components();
}

} // namespace diffusion {