#include "BTSystem.h"

#include "../BaseComponents/BTComponent.h"

BTSystem::BTSystem(::entt::registry& registry) : m_registry(registry)
{ 
}

void BTSystem::tick(int deltaSeconds)
{
    auto AITasks = m_registry.view<BTComponent>();
    AITasks.each([this,deltaSeconds](BTComponent& BT)
    {
        BT.root->tick(deltaSeconds);
    });
}
