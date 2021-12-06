#include "ComponentInitializer.h"

#include "Engine.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/ScriptComponent.h"

namespace diffusion {

ComponentInitializer::ComponentInitializer(Game& game)
	: m_game(game)
{
	m_game.get_registry().on_construct<Relation>().connect<&ComponentInitializer::add_childs_component>(*this);
	m_game.get_registry().on_construct<ScriptComponent>().connect<&ComponentInitializer::add_state>(*this);
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

void ComponentInitializer::add_state(::entt::registry& registry, ::entt::entity parent_entity)
{
	registry.emplace<ScriptComponentState>(parent_entity, diffusion::create_lua_state(registry));
}

void ComponentInitializer::add_to_execution(::entt::registry& registry, ::entt::entity parent_entity)
{
	m_game.get_tasks().push_back(m_game.get_taskflow().emplace(  // create four tasks
		[this, &registry, parent_entity]() {
			const auto& script = registry.get<ScriptComponent>(parent_entity);
			auto * lua_script = registry.try_get<ScriptComponentState>(parent_entity);
			if (lua_script) {
				registry.emplace<ScriptComponentState>(parent_entity, diffusion::create_lua_state(registry));
			}

			// state is local
			const int ret = luaL_dostring(lua_script->m_state, script.m_content.c_str());
			if (ret != LUA_OK) {
				const char* str = lua_tostring(lua_script->m_state, -1);
				lua_pop(lua_script->m_state, 1);
			}
		}
	));
}

} // namespace diffusion {