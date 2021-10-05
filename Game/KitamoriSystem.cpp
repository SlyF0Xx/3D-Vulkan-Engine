#include "KitamoriSystem.h"

#include "ComponentManager.h"

namespace diffusion {

KitamoriSystem::KitamoriSystem(BoundingComponent* initial)
    : System({}), m_linked_components({ initial })
    //: System({ BoundingComponent::s_bounding_component_tag })
{
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