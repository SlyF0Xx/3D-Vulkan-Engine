#include "PlaneEntity.h"

namespace diffusion {

PlaneEntity::PlaneEntity(Game& game)
    : PrimitiveEntity(
        game,
        {
          PrimitiveColoredVertex{-3.0,   0.0, -3.0,   {0.0f, 0.0f}},
          PrimitiveColoredVertex{-3.0,   0.0,  3.0,   {0.0f, 1.0f}},
          PrimitiveColoredVertex{ 3.0,   0.0, -3.0,   {1.0f, 0.0f}},
          PrimitiveColoredVertex{ 3.0,   0.0,  3.0,   {1.0f, 1.0f}}
        },
        {
          0, 1, 3,
          0, 3, 2
        },
        { 0, -3.0, 0 },
        { 0, 0, 0 },
        { 10, 10, 1 }
    )
{
}

} // namespace diffusion {