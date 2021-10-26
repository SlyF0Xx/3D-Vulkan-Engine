#include "PrimitiveEntity.h"

#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"
#include "VulkanMeshComponentManager.h"

#include "UnlitMaterial.h"
#include "LitMaterial.h"

namespace diffusion {

namespace entt {

::entt::entity create_primitive_entity_base(
	::entt::registry& registry,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale)
{
	auto entity = registry.create();
	registry.emplace<SubMesh>(entity, verticies, indexes);
	registry.emplace<TransformComponent>(entity, create_matrix(position, rotation, scale));
	return entity;
}

void add_default_unlit_material_component(::entt::registry& registry, ::entt::entity parent_entity)
{
	registry.emplace<UnlitMaterialComponent>(parent_entity, "default.png");
}

void add_default_lit_material_component(::entt::registry& registry, ::entt::entity parent_entity)
{
	registry.emplace<LitMaterialComponent>(parent_entity, "default.png", "red.png");
}

}

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