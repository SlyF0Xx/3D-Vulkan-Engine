#pragma once

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}

#include <LuaBridge/LuaBridge.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

using namespace luabridge;

#include <string>

namespace diffusion {

struct ScriptComponent {
	std::string m_content;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ScriptComponent, m_content)
};

struct ScriptComponentState {
	lua_State* m_state;
};

lua_State* create_lua_state(entt::registry & registry);

}