#include "Entity.h"

namespace diffusion {

std::vector<std::reference_wrapper<Component>> Entity::get_components()
{
    std::vector<std::reference_wrapper<Component>> components;
    for (auto& component : m_components) {
        components.push_back(component.get_component());
    }
    return components;
}

Component& Entity::add_component(ComponentGuard component)
{
    //m_components.emplace_back(std::move(component));
    m_components.push_back(std::move(component));
    return m_components.back().get_component();
}

} // namespace diffusion {