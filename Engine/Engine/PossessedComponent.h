#pragma once
#include "Component.h"

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

class PossessedComponent :
    public Component
{
public:
	PossessedComponent(const std::vector<Tag>& tags, Entity* parent);

	static inline Component::Tag s_possessed_tag;
};

} // namespace diffusion {