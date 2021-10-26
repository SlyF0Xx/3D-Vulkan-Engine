#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

struct Relation
{
	::entt::entity m_parent;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Relation, m_parent)
};

}

}