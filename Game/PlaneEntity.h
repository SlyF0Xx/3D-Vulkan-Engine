#pragma once
#include "PrimitiveEntity.h"
#include "Engine.h"

namespace diffusion {

class PlaneEntity :
    public PrimitiveEntity
{
public:
    PlaneEntity(Game& game, glm::vec3 translation = { 0, 0, 0}, glm::vec3 rotation = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 });
};

class PlaneLitEntity :
    public PrimitiveLitEntity
{
public:
    PlaneLitEntity(Game& game, glm::vec3 translation = { 0, 0, 0 }, glm::vec3 rotation = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 });
};


} // namespace diffusion {