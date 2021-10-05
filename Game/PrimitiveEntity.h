#pragma once

#include "Entity.h"
#include "Engine.h"

namespace diffusion {

class PrimitiveEntity :
    public Entity
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

} // namespace diffusion {