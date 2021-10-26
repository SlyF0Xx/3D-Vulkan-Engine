#include "BoundingComponent.h"
#include "TransformComponent.h"

namespace diffusion {

namespace entt {

bool intersect(::entt::registry& registry,
	const BoundingComponent& lhv,
	const BoundingComponent& rhv,
	const TransformComponent& lhv_transform,
	const TransformComponent& rhv_transform)
{
	glm::mat4 lhv_world_mat = calculate_global_world_matrix(registry, lhv_transform);
	glm::mat4 rhv_world_mat = calculate_global_world_matrix(registry, rhv_transform);

	return glm::length(glm::vec4(lhv_world_mat * glm::vec4(lhv.m_center, 1.0f)) - glm::vec4(rhv_world_mat * glm::vec4(rhv.m_center, 1.0f))) <= (lhv.m_radius + rhv.m_radius);
}

bool intersect(::entt::registry& registry,
	const BoundingComponent& lhv,
	const BoundingComponent& rhv)
{
	const auto& lhv_transform = registry.get<const TransformComponent>(::entt::to_entity(registry, lhv));
	const auto& rhv_transform = registry.get<const TransformComponent>(::entt::to_entity(registry, rhv));
	return intersect(registry, lhv, rhv, lhv_transform, rhv_transform);
}

}

}