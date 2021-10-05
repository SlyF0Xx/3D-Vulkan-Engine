#pragma once

#include "PrimitiveEntity.h"
#include "Engine.h"

namespace diffusion{

class CubeEntity :
    public PrimitiveEntity
{
public:
    CubeEntity(Game& game, glm::vec3 translation);
};

} // namespace diffusion {