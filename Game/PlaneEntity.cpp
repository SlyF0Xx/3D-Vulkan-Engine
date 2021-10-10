#include "PlaneEntity.h"

namespace diffusion {

PlaneEntity::PlaneEntity(Game& game, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : PrimitiveEntity(
        game,
        {
          PrimitiveColoredVertex{-0.5,   0.0, -0.5,   {0.0f, 0.0f}},
          PrimitiveColoredVertex{-0.5,   0.0,  0.5,   {0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,   0.0, -0.5,   {1.0f, 0.0f}},
          PrimitiveColoredVertex{ 0.5,   0.0,  0.5,   {1.0f, 1.0f}}
        },
        {
          0, 1, 3,
          0, 3, 2
        },
        translation,
        rotation,
        scale
    )
{
}

PlaneLitEntity::PlaneLitEntity(Game& game, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : PrimitiveLitEntity(
        game,
        {
          PrimitiveColoredVertex{-0.5,   0.0, -0.5,   {0.0f, 0.0f}},
          PrimitiveColoredVertex{-0.5,   0.0,  0.5,   {0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,   0.0, -0.5,   {1.0f, 0.0f}},
          PrimitiveColoredVertex{ 0.5,   0.0,  0.5,   {1.0f, 1.0f}}
        },
        {
          0, 1, 3,
          0, 3, 2
        },
        translation,
        rotation,
        scale
    )
{
}

} // namespace diffusion {