#pragma once
#include "PrimitiveEntity.h"
#include "Engine.h"

namespace diffusion {

class PlaneEntity :
    public PrimitiveEntity
{
public:
    PlaneEntity(Game& game);
};

} // namespace diffusion {