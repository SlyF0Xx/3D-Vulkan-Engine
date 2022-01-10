#include "ComponentInitializer.h"

#include <chrono>

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

			if (!registry.try_get<ScriptComponentState>(parent_entity)) {
				registry.emplace<ScriptComponentState>(parent_entity, diffusion::create_lua_state(registry));
			}
			const ScriptComponentState& lua_script = registry.get<ScriptComponentState>(parent_entity);

			// state is local
			const int ret = luaL_dostring(lua_script.m_state, script.m_content.c_str());
			if (ret != LUA_OK) {
				const char* str = lua_tostring(lua_script.m_state, -1);
				std::cerr << str << std::endl;
				lua_pop(lua_script.m_state, 1);
			}

			auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			if (!lua_script.m_is_initialized) {
				//luaL_dostring(lua_script.m_state, script.m_content.c_str());
				//lua_pcall(lua_script.m_state, 0, 0, 0);

				LuaRef func = luabridge::getGlobal(lua_script.m_state, "on_start");
				auto ret = func(time);
				if (!ret.wasOk()) {
					std::string err = ret.errorMessage();
					std::cerr << err;
				}

				registry.patch<ScriptComponentState>(parent_entity, [&](ScriptComponentState& state) {
					state.m_is_initialized = true;
				});
			}

			{
				LuaRef func = luabridge::getGlobal(lua_script.m_state, "on_update");
				LuaResult ret = func(time);
				if (!ret.wasOk()) {
					std::string err = ret.errorMessage();
					std::cerr << err;
				}
			}

			if (m_game.m_BehaviourTreeSystem) {
				m_game.m_BehaviourTreeSystem->tick(
					std::chrono::duration_cast<std::chrono::milliseconds>(m_game.m_script_time_point.time_since_epoch()).count() - time
				);
			}
		}
	));
}

} // namespace diffusion {