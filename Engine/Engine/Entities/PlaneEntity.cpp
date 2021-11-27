#include "PlaneEntity.h"

namespace diffusion {

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
        scale,
        {
            glm::vec3(-0.5, -0.5, 0.0),
            glm::vec3( 0.5,  0.5, 0.0)
        });
}

::entt::entity create_plane_entity_lit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto plane_entity = create_plane_entity(registry, translation, rotation, scale);
    diffusion::add_default_lit_material_component(registry, plane_entity);
    return plane_entity;
}

::entt::entity create_plane_entity_unlit(::entt::registry& registry, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    auto plane_entity = create_plane_entity(registry, translation, rotation, scale);
    add_default_unlit_material_component(registry, plane_entity);
    return plane_entity;
}

} // namespace diffusion {