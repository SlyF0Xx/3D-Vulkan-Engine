#include "DirectionalLightEntity.h"

#include "../BaseComponents/CameraComponent.h"
#include "../BaseComponents/DirectionalLightComponent.h"
#include "../BaseComponents/PointLightComponent.h"
#include "../BaseComponents/TransformComponent.h"
#include "../Engine.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace diffusion {

::entt::entity create_directional_light_entity(::entt::registry& registry, const glm::vec3& position, const glm::vec3& rotation/*, const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector*/)
{
	auto entity = registry.create();
	// TODO: ortho projection
	registry.emplace<TransformComponent>(entity, create_matrix(position, rotation, glm::vec3(1)));
	registry.emplace<CameraComponent>(entity/*, position, cameraTarget, upVector*//*, glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f)*/);
	registry.emplace<DirectionalLightComponent>(entity);
	return entity;
}

::entt::entity create_point_light_entity(::entt::registry& registry, const glm::vec3& position)
{
	auto entity = registry.create();
	// TODO: ortho projection
	registry.emplace<TransformComponent>(entity, create_matrix(position, glm::vec3(0), glm::vec3(1)));
	registry.emplace<PointLightComponent>(entity /*, glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f)*/);
	return entity;
}

} // namespace diffusion {