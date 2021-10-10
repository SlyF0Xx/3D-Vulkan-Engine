#include "PrimitiveEntity.h"

#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"
#include "VulkanMeshComponentManager.h"

namespace diffusion {

std::vector<ComponentGuard> create_components(
	Game& game,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	Entity* parent)
{
	std::vector<ComponentGuard> comp_vec;
	comp_vec.push_back(ComponentGuard(std::make_unique<VulkanMeshComponent>(game, verticies, indexes, std::vector<Component::Tag>{}, parent)));
	comp_vec.push_back(ComponentGuard(std::make_unique<VulkanTransformComponent>(game, position, rotation, scale, std::vector<Component::Tag>{}, parent)));
	return comp_vec;
}

PrimitiveEntityBase::PrimitiveEntityBase(
	Game& game,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
	: Entity(create_components(game, verticies, indexes, position, rotation, scale, this))
{
}

PrimitiveEntity::PrimitiveEntity(
	Game& game,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
	: PrimitiveEntityBase(game, verticies, indexes, position, rotation, scale)
{
	s_vulkan_mesh_component_manager.register_mesh(0, static_cast<MeshComponent*>(&get_components()[0].get()));
}

PrimitiveLitEntity::PrimitiveLitEntity(
	Game& game,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
	: PrimitiveEntityBase(game, verticies, indexes, position, rotation, scale)
{
	s_vulkan_mesh_component_manager.register_mesh(1, static_cast<MeshComponent*>(&get_components()[0].get()));
}

} // namespace diffusion {