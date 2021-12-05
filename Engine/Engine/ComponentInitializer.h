#pragma once

#include <entt/entt.hpp>

class Game;

namespace diffusion {

class ComponentInitializer
{
public:
	ComponentInitializer(Game& game);

	void add_childs_component(::entt::registry& registry, ::entt::entity parent_entity);
	void add_state(::entt::registry& registry, ::entt::entity parent_entity);
	void add_to_execution(::entt::registry& registry, ::entt::entity parent_entity);

private:
    Game& m_game;
};

} // namespace diffusion {