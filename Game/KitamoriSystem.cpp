#include "KitamoriSystem.h"

#include "ComponentManager.h"
#include "Entity.h"
#include "PossessedComponent.h"
#include "TransformComponent.h"

namespace diffusion {

KitamoriSystem::KitamoriSystem(::entt::registry& registry)
    : /*System({}), */m_registry(registry)
    //: System({ BoundingComponent::s_bounding_component_tag })
{
    m_registry.emplace<entt::KitamoriLinkedTag>(m_registry.ctx<diffusion::entt::PossessedEntity>().m_entity);

    /*
    for (auto& component : s_component_manager_instance.get_components_by_tag(PossessedComponent::s_possessed_tag)) {
        for (auto& inner_component : component.get().get_parrent()->get_components()) {
            auto it = std::find(
                inner_component.get().get_tags().begin(),
                inner_component.get().get_tags().end(),
                diffusion::BoundingComponent::s_bounding_component_tag);
            if (it != inner_component.get().get_tags().end()) {
                auto& comp = dynamic_cast<diffusion::BoundingComponent&>(inner_component.get());
                m_linked_components.push_back(static_cast<BoundingComponent*>(&comp));
            }
        }
    }
    */
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

    /*
    std::vector<::entt::entity> potential_linked_components;

    std::vector<BoundingComponent*> potential_linked_components;
    auto components = s_component_manager_instance.get_components_by_tag(BoundingComponent::s_bounding_component_tag);
    for (auto& component : components) {
        auto bounding = static_cast<BoundingComponent*>(&component.get());
        if (std::find(m_linked_components.begin(), m_linked_components.end(), bounding) == m_linked_components.end()) {
            potential_linked_components.push_back(bounding);
        }
    }

    for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
        if (m_linked_components.front()->Intersect(**it)) {
            m_linked_components.push_back(*it);
            it = potential_linked_components.erase(it);
        }
        else {
            ++it;
        }
    }
    */
}

} // namespace diffusion {