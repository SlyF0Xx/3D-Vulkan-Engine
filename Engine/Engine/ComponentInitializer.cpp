#include "ComponentInitializer.h"

#include "Engine.h"
#include "Relation.h"

namespace diffusion {

ComponentInitializer::ComponentInitializer(Game& game)
	: m_game(game)
{
	m_game.get_registry().on_construct<Relation>().connect<&ComponentInitializer::add_childs_component>(*this);
}

void ComponentInitializer::add_childs_component(::entt::registry& registry, ::entt::entity parent_entity)
{
	const auto & relation = registry.get<Relation>(parent_entity);

	auto * childs = registry.try_get<Childs>(relation.m_parent);

	if (childs) {
		childs->m_childs.insert(parent_entity);
	}
	else {
		auto & constructed_childs = registry.emplace<Childs>(relation.m_parent);
		constructed_childs.m_childs.insert(parent_entity);
	}
}

} // namespace diffusion {