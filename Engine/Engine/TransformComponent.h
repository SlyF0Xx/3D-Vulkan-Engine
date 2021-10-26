#pragma once

#include "Component.h"
#include "glm_printer.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

namespace entt {

struct TransformComponent
{
	glm::mat4 m_world_matrix;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TransformComponent, m_world_matrix)
};

glm::mat4 calculate_global_world_matrix(::entt::registry & registry, const TransformComponent & component);

glm::mat4 create_matrix(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale);

}

class TransformComponent : public Component
{
public:
	inline static Tag s_transform_component_tag;

	TransformComponent(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const std::vector<Tag>& tags,
		Entity* parent);

	virtual void UpdateWorldMatrix(const glm::mat4& world_matrix);

	glm::mat4 get_world_matrix() const
	{
		return m_world_matrix;
	}

private:
	glm::mat4 m_world_matrix;
};

} // namespace diffusion {