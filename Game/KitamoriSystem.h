#pragma once
#include "System.h"
#include "BoundingComponent.h"

namespace diffusion {

class KitamoriSystem :
    public System
{
public:
    KitamoriSystem();
    void update_components();

protected:
    std::vector<BoundingComponent*> m_linked_components;
};

} // namespace diffusion {