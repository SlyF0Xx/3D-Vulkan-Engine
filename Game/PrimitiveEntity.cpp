#include "PrimitiveEntity.h"

#include "VulkanTransformComponent.h"
#include "VulkanMeshComponent.h"

#include "UnlitMaterial.h"
#include "LitMaterial.h"

namespace diffusion {

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

} // namespace diffusion {