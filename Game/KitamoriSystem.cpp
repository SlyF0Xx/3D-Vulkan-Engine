#include "KitamoriSystem.h"

#include <edyn/collision/contact_manifold.hpp>


#include "PossessedComponent.h"
#include "TransformComponent.h"

namespace diffusion {

KitamoriSystem::KitamoriSystem(::entt::registry& registry)
    : m_registry(registry)
{
    m_registry.emplace<KitamoriLinkedTag>(m_registry.ctx<diffusion::PossessedEntity>().m_entity);
}

void KitamoriSystem::update_components()
{
    auto & possesed_entity = m_registry.ctx<diffusion::PossessedEntity>().m_entity;
    const auto & possesed_bounding_component = m_registry.get<const BoundingComponent>(possesed_entity);
    const auto & possesed_transform_component = m_registry.get<const TransformComponent>(possesed_entity);

    auto potential_linked_components = m_registry.view<const BoundingComponent, const TransformComponent>(::entt::exclude<KitamoriLinkedTag>);
   /* potential_linked_components.each([this, &possesed_bounding_component, &possesed_transform_component]
        (const BoundingComponent & bounding_component, const TransformComponent & transform_component) {

        if (intersect(m_registry, possesed_bounding_component, bounding_component, possesed_transform_component, transform_component)) {
            m_registry.emplace<KitamoriLinkedTag>(::entt::to_entity(m_registry, bounding_component));
        }
    });*/
/*
    auto linkeds = m_registry.view<const edyn::contact_manifold>();
    linkeds.each([this, &possesed_bounding_component, &possesed_transform_component]
        (const edyn::contact_manifold &contact)
    {
        contact.body
    });*/
}

} // namespace diffusion {