#include "TransformComponent.h"

#include "Relation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace diffusion {

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

glm::mat4 create_matrix_by_location_target(
	const glm::vec3& position,
	const glm::vec3& target)
{
	const glm::vec3 view_vector = glm::normalize(target - position);

	auto quat = glm::rotation(view_vector, glm::vec3(0, 1, 0));
	glm::vec3 euler = glm::eulerAngles(quat);

	return create_matrix(position, glm::vec3(-euler.x, -euler.y, -euler.z));
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
	rotation_matrix = glm::rotate(glm::mat4(1), rotation[1], RotationY) * rotation_matrix;

	glm::vec3 RotationZ(0, 0, 1.0);
	rotation_matrix = glm::rotate(glm::mat4(1), rotation[2], RotationZ) * rotation_matrix;

	glm::mat4 scale_matrix = glm::scale(scale);
	return translation_matrix * rotation_matrix * scale_matrix;
}

} // namespace diffusion {