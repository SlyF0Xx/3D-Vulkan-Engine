#include "BoundingSphere.h"

bool does_intersect(const BoundingSphere& lhv, const BoundingSphere& rhv)
{
    return glm::length(rhv.center - lhv.center) <= (lhv.radius + rhv.radius);
}
