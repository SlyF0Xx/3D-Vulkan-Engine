#include "CubeEntity.h"

#include "BoundingComponent.h"
#include "TransformComponent.h"
#include "PossessedComponent.h"
#include "VulkanCameraComponent.h"

namespace diffusion {

::entt::entity create_cube_entity(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
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
            2, 1, 0,
            2, 3, 1,

            // left
            5, 4, 0,
            1, 5, 0,
            
            // right
            2, 6, 7,
            2, 7, 3,

            // backward
            6, 4, 5,
            6, 5, 7,

            // down
            7, 5, 1,
            3, 7, 1,

            // top
            0, 4, 6,
            0, 6, 2
        },
        translation,
        rotation,
        scale);
}

::entt::entity create_cube_entity_lit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto cube_entity = create_cube_entity(registry, translation, rotation, scale);
    diffusion::add_default_lit_material_component(registry, cube_entity);
    return cube_entity;
}

::entt::entity create_cube_entity_unlit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto cube_entity = create_cube_entity(registry, translation, rotation, scale);
    add_default_unlit_material_component(registry, cube_entity);
    registry.emplace<CameraComponent>(cube_entity);
    registry.emplace<BoundingComponent>(cube_entity, glm::vec3(-0.5f, 0.5f, 0.6f), 0.25f);

    return cube_entity;
}

} // namespace diffusion {