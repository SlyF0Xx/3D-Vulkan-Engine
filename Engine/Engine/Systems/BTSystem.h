#pragma once
#include <entt/entt.hpp>

class BTSystem
{
public:
    BTSystem(::entt::registry& registry);
    void tick(int deltaSeconds);
private:
    ::entt::registry & m_registry;
};
