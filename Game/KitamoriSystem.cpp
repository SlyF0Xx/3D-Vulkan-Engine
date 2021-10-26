#include "KitamoriSystem.h"

#include "PossessedComponent.h"
#include "TransformComponent.h"

namespace diffusion {

KitamoriSystem::KitamoriSystem(::entt::registry& registry)
    : m_registry(registry)
{
    m_registry.emplace<entt::KitamoriLinkedTag>(m_registry.ctx<diffusion::entt::PossessedEntity>().m_entity);
}

void KitamoriSystem::update_components()
{
    auto & possesed_entity = m_registry.ctx<diffusion::entt::PossessedEntity>().m_entity;
    const auto & possesed_bounding_component = m_registry.get<const entt::BoundingComponent>(possesed_entity);
    const auto & possesed_transform_component = m_registry.get<const entt::TransformComponent>(possesed_entity);

    auto potential_linked_components = m_registry.view<const entt::BoundingComponent, const entt::TransformComponent>(::entt::exclude<entt::KitamoriLinkedTag>);
    potential_linked_components.each([this, &possesed_bounding_component, &possesed_transform_component]
        (const entt::BoundingComponent & bounding_component, const entt::TransformComponent & transform_component) {

        if (entt::intersect(m_registry, possesed_bounding_component, bounding_component, possesed_transform_component, transform_component)) {
            m_registry.emplace<entt::KitamoriLinkedTag>(::entt::to_entity(m_registry, possesed_bounding_component));
        }
    });
}

} // namespace diffusion {