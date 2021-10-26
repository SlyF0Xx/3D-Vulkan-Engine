#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

struct PossessedEntity
{
	::entt::entity m_entity;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PossessedEntity, m_entity)
};

}

} // namespace diffusion {