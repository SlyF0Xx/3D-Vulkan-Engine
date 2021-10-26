#include "PlaneEntity.h"

namespace diffusion {

namespace entt {

::entt::entity create_plane_entity(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    return create_primitive_entity_base(
        registry,
        {
            PrimitiveColoredVertex{-0.5,  -0.5, 0.0,   {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            PrimitiveColoredVertex{-0.5,   0.5, 0.0,   {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            PrimitiveColoredVertex{ 0.5,  -0.5, 0.0,   {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            PrimitiveColoredVertex{ 0.5,   0.5, 0.0,   {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
        },
        {
            0, 3, 1,
            0, 2, 3
        },
        translation,
        rotation,
        scale);
}

::entt::entity create_plane_entity_lit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto plane_entity = create_plane_entity(registry, translation, rotation, scale);
    diffusion::entt::add_default_lit_material_component(registry, plane_entity);
    return plane_entity;
}

::entt::entity create_plane_entity_unlit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto plane_entity = create_plane_entity(registry, translation, rotation, scale);
    add_default_unlit_material_component(registry, plane_entity);
    return plane_entity;
}

}

PlaneEntity::PlaneEntity(Game& game, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : PrimitiveEntity(
        game,
        {
          PrimitiveColoredVertex{-0.5,  -0.5, 0.0,   {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{-0.5,   0.5, 0.0,   {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,  -0.5, 0.0,   {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,   0.5, 0.0,   {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
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
          PrimitiveColoredVertex{-0.5,  -0.5, 0.0,   {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{-0.5,   0.5, 0.0,   {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,  -0.5, 0.0,   {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5,   0.5, 0.0,   {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
        },
        {
          3, 1, 0,
          2, 3, 0
        },
        translation,
        rotation,
        scale
    )
{
}

} // namespace diffusion {