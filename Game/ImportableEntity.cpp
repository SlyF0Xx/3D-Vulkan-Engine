#include "ImportableEntity.h"

#include "ImportableVulkanMeshComponents.h"
#include "VulkanTransformComponent.h"

namespace diffusion {

namespace entt {

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

}

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

std::vector<ComponentGuard> create_cat_components(Game& game, const std::filesystem::path& path, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, Entity* parent)
{
	std::vector<ComponentGuard> comp_vec;
	comp_vec.push_back(ComponentGuard(std::make_unique<ImportableVulkanMeshComponents>(game, path, std::vector<Component::Tag>{}, parent)));
	comp_vec.push_back(ComponentGuard(std::make_unique<VulkanTransformComponent>(game, position, rotation, scale, std::vector<Component::Tag>{CatImportableEntity::s_special_cat_transform_tag}, parent)));
	return comp_vec;
}

CatImportableEntity::CatImportableEntity(Game& game, const std::filesystem::path& path, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
	: Entity(create_cat_components(game, path, position, rotation, scale, this))
{
}

} // namespace diffusion {