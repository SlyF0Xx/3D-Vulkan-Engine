#include "TransformComponent.h"

#include "Relation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

namespace entt {

glm::mat4 calculate_global_world_matrix(::entt::registry& registry, const TransformComponent& component)
{
	auto * relation_component = registry.try_get<Relation>(::entt::to_entity(registry, component));
	if (relation_component) {
		auto* parent_transform_component = registry.try_get<TransformComponent>(relation_component->m_parent);
		if (parent_transform_component) {
			return calculate_global_world_matrix(registry, *parent_transform_component) * component.m_world_matrix;
		}
	}

	return component.m_world_matrix;
}

glm::mat4 create_matrix(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
{
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1), position);

	glm::mat4 rotation_matrix(1);

	glm::vec3 RotationX(1.0, 0, 0);
	rotation_matrix = glm::rotate(glm::mat4(1), rotation[0], RotationX);

	glm::vec3 RotationY(0, 1.0, 0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[1], RotationY);

	glm::vec3 RotationZ(0, 0, 1.0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[2], RotationZ);

	glm::mat4 scale_matrix = glm::scale(scale);
	return translation_matrix * rotation_matrix * scale_matrix;
}

}

TransformComponent::TransformComponent(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const std::vector<Tag>& tags,
	Entity* parent)
	: Component(concat_vectors({ s_transform_component_tag }, tags), parent)
{
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1), position);

	glm::mat4 rotation_matrix(1);

	glm::vec3 RotationX(1.0, 0, 0);
	rotation_matrix = glm::rotate(glm::mat4(1), rotation[0], RotationX);

	glm::vec3 RotationY(0, 1.0, 0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[1], RotationY);

	glm::vec3 RotationZ(0, 0, 1.0);
	rotation_matrix *= glm::rotate(glm::mat4(1), rotation[2], RotationZ);

	glm::mat4 scale_matrix = glm::scale(scale);

	m_world_matrix = translation_matrix * rotation_matrix * scale_matrix;
}

void TransformComponent::UpdateWorldMatrix(const glm::mat4& world_matrix)
{
	m_world_matrix = world_matrix;
}

} // namespace diffusion {