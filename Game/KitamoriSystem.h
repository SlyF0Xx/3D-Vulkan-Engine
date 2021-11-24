#pragma once

#include "BaseComponents/BoundingComponent.h"

#include <entt/entt.hpp>

namespace diffusion {

__declspec(align(2)) struct KitamoriLinkedTag
{};

class KitamoriSystem
{
public:
    KitamoriSystem(::entt::registry& registry);
    void update_components();

protected:
    ::entt::registry& m_registry;
};

} // namespace diffusion {