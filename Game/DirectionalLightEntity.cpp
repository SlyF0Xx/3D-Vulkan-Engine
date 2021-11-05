#include "DirectionalLightEntity.h"

#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "Engine.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace diffusion {

::entt::entity create_directional_light_entity(::entt::registry& registry, const glm::vec3& position, const glm::vec3& cameraTarget, const glm::vec3& upVector)
{
	auto entity = registry.create();
	// TODO: ortho projection
	registry.emplace<CameraComponent>(entity, position, cameraTarget, upVector/*, glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f)*/);
	registry.emplace<DirectionalLightComponent>(entity);
	return entity;
}

} // namespace diffusion {