#pragma once

#include "export.h"

// Actually, DescriptorSet - different textures
class ENGINE_API IMaterial
{
    int id;
public:
    IMaterial();
    virtual ~IMaterial() = default;

    virtual void UpdateMaterial() = 0;
    int get_id() { return id; };
};

enum class ENGINE_API MaterialType
{
    Opaque
};
