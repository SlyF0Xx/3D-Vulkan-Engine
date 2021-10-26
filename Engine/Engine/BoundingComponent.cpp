#include "BoundingComponent.h"
#include "Entity.h"
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

BoundingComponent::BoundingComponent(
	glm::vec3 center,
	float radius,
	const std::vector<Tag>& tags,
	Entity* parent)
	: Component(concat_vectors({ s_bounding_component_tag }, tags), parent), m_center(center), m_radius(radius)
{
}

bool BoundingComponent::Intersect(const BoundingComponent& other)
{
	glm::mat4 own_world_mat;
	for (auto& inner_component : get_parrent()->get_components()) {
		auto it = std::find(
			inner_component.get().get_tags().begin(),
			inner_component.get().get_tags().end(),
			diffusion::TransformComponent::s_transform_component_tag);
		if (it != inner_component.get().get_tags().end()) {
			auto& comp = dynamic_cast<diffusion::TransformComponent&>(inner_component.get());
			own_world_mat = comp.get_world_matrix();
		}
	}
	
	glm::mat4 other_world_mat;
	for (auto& inner_component : other.get_parrent()->get_components()) {
		auto it = std::find(
			inner_component.get().get_tags().begin(),
			inner_component.get().get_tags().end(),
			diffusion::TransformComponent::s_transform_component_tag);
		if (it != inner_component.get().get_tags().end()) {
			auto& comp = dynamic_cast<diffusion::TransformComponent&>(inner_component.get());
			other_world_mat = comp.get_world_matrix();
		}
	}

	return glm::length(glm::vec4(own_world_mat * glm::vec4(other.m_center, 1.0f)) - glm::vec4(other_world_mat * glm::vec4(m_center, 1.0f))) <= (m_radius + other.m_radius);
}

}