#include "TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

TransformComponent::TransformComponent(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const std::vector<Tag>& tags)
	: Component(concat_vectors({ s_transform_component_tag }, tags))
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