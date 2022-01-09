#pragma once
#include <entt/entt.hpp>

class BTSystem
{
public:
    BTSystem(::entt::registry& registry);
    ~BTSystem();
    void tick(float deltaSeconds);
private:
    ::entt::registry & m_registry;
};
