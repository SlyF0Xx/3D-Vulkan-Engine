#pragma once

#include "Entity.h"
#include "Engine.h"

namespace diffusion {

namespace entt {

::entt::entity create_primitive_entity_base(
	::entt::registry & registry,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale);

void add_default_unlit_material_component(::entt::registry& registry, ::entt::entity parent_entity);
void add_default_lit_material_component(::entt::registry& registry, ::entt::entity parent_entity);

}

class PrimitiveEntityBase :
	public Entity
{
public:
	PrimitiveEntityBase(
		Game& game,
		const std::vector<PrimitiveColoredVertex>& verticies,
		const std::vector<uint32_t>& indexes,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);
};

class PrimitiveEntity :
    public PrimitiveEntityBase
{
public:
	PrimitiveEntity(
		Game& game,
		const std::vector<PrimitiveColoredVertex>& verticies,
		const std::vector<uint32_t>& indexes,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);
};

class PrimitiveLitEntity :
	public PrimitiveEntityBase
{
public:
	PrimitiveLitEntity(
		Game& game,
		const std::vector<PrimitiveColoredVertex>& verticies,
		const std::vector<uint32_t>& indexes,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale);
};

} // namespace diffusion {