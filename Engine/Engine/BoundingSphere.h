#pragma once

#include "export.h"

#include <glm/glm.hpp>

struct ENGINE_API BoundingSphere
{
	glm::vec3 center;
	float radius;
};

bool ENGINE_API does_intersect(const BoundingSphere& lhv, const BoundingSphere& rhv);
