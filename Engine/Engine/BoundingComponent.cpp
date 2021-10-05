#include "BoundingComponent.h"
#include "Entity.h"
#include "TransformComponent.h"

namespace diffusion {

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