#include "PrimitiveEntity.h"

#include "../BaseComponents/VulkanComponents/VulkanTransformComponent.h"
#include "../BaseComponents/VulkanComponents/VulkanMeshComponent.h"
#include "../BaseComponents/Relation.h"

#include "../BaseComponents/UnlitMaterial.h"
#include "../BaseComponents/LitMaterial.h"

namespace diffusion {

::entt::entity create_primitive_entity_base(
	::entt::registry& registry,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const SubMesh::AABB& bounding_box)
{
	auto entity = registry.create();
	registry.emplace<SubMesh>(entity, verticies, indexes, bounding_box);
	registry.emplace<TransformComponent>(entity, create_matrix(position, rotation, scale));
	return entity;
}

void add_default_unlit_material_component(::entt::registry& registry, ::entt::entity parent_entity)
{
	registry.emplace<UnlitMaterialComponent>(parent_entity, "default.png");
}

void add_default_lit_material_component(::entt::registry& registry, ::entt::entity parent_entity)
{
	registry.emplace<LitMaterialComponent>(parent_entity, "default.png", "blue.png");
}

} // namespace diffusion {