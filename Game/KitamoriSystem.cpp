#include "KitamoriSystem.h"

#include "ComponentManager.h"
#include "Entity.h"
#include "PossessedComponent.h"

namespace diffusion {

KitamoriSystem::KitamoriSystem()
    : System({})
    //: System({ BoundingComponent::s_bounding_component_tag })
{
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
}

void KitamoriSystem::update_components()
{
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
}

} // namespace diffusion {