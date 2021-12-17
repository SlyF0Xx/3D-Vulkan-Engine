#pragma once
#include "KitamoriSystem.h"

#include <glm/glm.hpp>

#include "PhysicsSystem.h"

namespace diffusion {

class KitamoriMovingSystem :
    public KitamoriSystem
{
public:
    using KitamoriSystem::KitamoriSystem;

    void update_position(glm::vec3 direction);
    PhysicsSystem* linkedSystem;
};

} // namespace diffusion {