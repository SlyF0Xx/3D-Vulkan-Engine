#pragma once

#include "../BaseComponents/TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

class Game;

namespace diffusion {

class PhysicsSystem
{
public:
    PhysicsSystem(Game& game);
        
    void update_transform(::entt::registry& registry, ::entt::entity parent_entity);
    void update_phys_component(::entt::registry& registry, ::entt::entity parent_entity);
    void on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity);

private:
    Game& m_game;
};

} // namespace diffusion {