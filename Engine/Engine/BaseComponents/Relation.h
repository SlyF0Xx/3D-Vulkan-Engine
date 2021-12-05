#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <unordered_set>

namespace diffusion {

struct Relation
{
	::entt::entity m_parent;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Relation, m_parent)
};

struct Childs
{
	std::unordered_set<::entt::entity> m_childs;
};

void destroy_entity(entt::registry& registry, entt::entity & entity);

}