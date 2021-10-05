#include "ImportableEntity.h"

#include "ImportableVulkanMeshComponents.h"
#include "VulkanTransformComponent.h"

namespace diffusion {

std::vector<ComponentGuard> create_components(Game& game, const std::filesystem::path& path, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, Entity * parent)
{
	std::vector<ComponentGuard> comp_vec;
	comp_vec.push_back(ComponentGuard(std::make_unique<ImportableVulkanMeshComponents>(game, path, std::vector<Component::Tag>{}, parent)));
	comp_vec.push_back(ComponentGuard(std::make_unique<VulkanTransformComponent>(game, position, rotation, scale, std::vector<Component::Tag>{}, parent)));
	return comp_vec;
}

ImportableEntity::ImportableEntity(Game& game, const std::filesystem::path& path, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
	: Entity(create_components(game, path, position, rotation, scale, this))
{
}

} // namespace diffusion {