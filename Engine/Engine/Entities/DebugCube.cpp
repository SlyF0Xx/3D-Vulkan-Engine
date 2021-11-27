#include "DebugCube.h"

#include "../BaseComponents/BoundingComponent.h"
#include "../BaseComponents/TransformComponent.h"
#include "../BaseComponents/PossessedComponent.h"
#include "../BaseComponents/VulkanComponents/VulkanCameraComponent.h"
#include "../BaseComponents/DebugComponent.h"

namespace diffusion {

::entt::entity create_debug_cube_entity_impl(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    return create_primitive_entity_base(
        registry,
        {
            PrimitiveColoredVertex{ 0.5f, -0.5f,  0.5f, {0.0f, 1.0f}},
            PrimitiveColoredVertex{ 0.5f, -0.5f, -0.5f, {0.0f, 0.0f}},
            PrimitiveColoredVertex{-0.5f, -0.5f,  0.5f, {1.0f, 1.0f}},
            PrimitiveColoredVertex{-0.5f, -0.5f, -0.5f, {1.0f, 0.0f}},

            PrimitiveColoredVertex{ 0.5f,  0.5f,  0.5f, {1.0f, 1.0f}},
            PrimitiveColoredVertex{ 0.5f,  0.5f, -0.5f, {1.0f, 0.0f}},
            PrimitiveColoredVertex{-0.5f,  0.5f,  0.5f, {0.0f, 1.0f}},
            PrimitiveColoredVertex{-0.5f,  0.5f, -0.5f, {0.0f, 0.0f}}
        },
        {
            // forward
            0, 1,
            1, 3,
            3, 2,
            2, 0,

            // right
            0, 4,
            4, 5,
            5, 1,

            // left
            2, 6,
            6, 7,
            7, 3,

            // backward
            6, 4,
            5, 7
        },
        translation,
        rotation,
        scale);
}

::entt::entity create_debug_cube_entity(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto cube_entity = create_debug_cube_entity_impl(registry, translation, rotation, scale);
    registry.emplace<debug_tag>(cube_entity);

    return cube_entity;
}

::entt::entity create_debug_sphere_entity_impl(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    const float radius = 1;
    const int sectorCount = 10;
    const int stackCount = 10;

    // clear memory of prev arrays
    std::vector<PrimitiveColoredVertex> vertices;

    float x, y, z, xy;                              // vertex position

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::pi<float>() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(PrimitiveColoredVertex({ x, y, z, {0, 0} }));
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    std::vector<uint32_t> lineIndices;
    uint32_t k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // store indices for lines
            // vertical lines for all stacks, k1 => k2
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }

    return create_primitive_entity_base(
        registry,
        vertices,
        lineIndices,
        translation,
        rotation,
        scale);
}

::entt::entity create_debug_sphere_entity(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto sphere_entity = create_debug_sphere_entity_impl(registry, translation, rotation, scale);
    registry.emplace<debug_tag>(sphere_entity);

    return sphere_entity;
}

} // namespace diffusion {