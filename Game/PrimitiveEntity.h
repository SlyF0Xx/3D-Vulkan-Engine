#pragma once

#include "Entity.h"
#include "Engine.h"

namespace diffusion {

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