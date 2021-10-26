#pragma once
#include "System.h"
#include "BoundingComponent.h"

#include <entt/entt.hpp>

namespace diffusion {

namespace entt {

__declspec(align(2)) struct KitamoriLinkedTag
{};

}

class KitamoriSystem /*:
    public System */
{
public:
    KitamoriSystem(::entt::registry& registry);
    void update_components();

protected:
    ::entt::registry& m_registry;
    //std::vector<BoundingComponent*> m_linked_components;
};

} // namespace diffusion {