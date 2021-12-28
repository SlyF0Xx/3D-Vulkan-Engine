#pragma once

#include "../BaseComponents/TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

namespace diffusion {

class PhysicsSystem
{
public:
    PhysicsSystem(::entt::registry& registry);
        
    void update_phys_component(::entt::registry& registry, ::entt::entity parent_entity);
    void on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity);
};

} // namespace diffusion {