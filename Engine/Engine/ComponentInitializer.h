#pragma once

#include <entt/entt.hpp>

class Game;

namespace diffusion {

class ComponentInitializer
{
public:
	ComponentInitializer(Game& game);

	void add_childs_component(::entt::registry& registry, ::entt::entity parent_entity);

private:
    Game& m_game;
};

} // namespace diffusion {