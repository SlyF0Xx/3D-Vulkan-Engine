#include "KitamoriMovingSystem.h"
#include "BaseComponents/TransformComponent.h"

#include "KitamoriSystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

namespace diffusion {

void KitamoriMovingSystem::update_position(glm::vec3 direction)
{
    auto potential_linked_components = m_registry.view<const KitamoriLinkedTag>();
    potential_linked_components.each([this, &direction](const KitamoriLinkedTag& tag) {
        m_registry.patch<TransformComponent>(::entt::to_entity(m_registry, tag), [&direction](TransformComponent& transform) {
            transform.m_world_matrix = glm::translate(transform.m_world_matrix, direction);
        });
    });

    update_components();
}

} // namespace diffusion {