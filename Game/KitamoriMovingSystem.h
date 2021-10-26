#pragma once
#include "KitamoriSystem.h"

#include <glm/glm.hpp>

namespace diffusion {

class KitamoriMovingSystem :
    public KitamoriSystem
{
public:
    using KitamoriSystem::KitamoriSystem;

    void update_position(glm::vec3 direction);
};

} // namespace diffusion {