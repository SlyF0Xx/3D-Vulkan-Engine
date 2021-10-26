#include "ImportableEntity.h"

#include "ImportableVulkanMeshComponents.h"
#include "VulkanTransformComponent.h"

namespace diffusion {

::entt::entity import_entity(
	::entt::registry& registry,
	const std::filesystem::path& path,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
{
	auto entity = registry.create();
	registry.emplace<TransformComponent>(entity, create_matrix(position, rotation, scale));
	import_mesh(path, registry, entity);
	return entity;
}

} // namespace diffusion {