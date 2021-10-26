#include "RotateSystem.h"

#include "ImportableEntity.h"

namespace diffusion {

RotateSystem::RotateSystem(::entt::registry& registry)
    : m_registry(registry)
	//: System({ CatImportableEntity::s_special_cat_transform_tag })
{
}

void RotateSystem::tick()
{
    auto& entity = m_registry.ctx<entt::RotateTag>().m_entity;
    m_registry.patch<entt::TransformComponent>(entity, [this] (entt::TransformComponent & transform) {
        rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
        transform.m_world_matrix = transform.m_world_matrix * rotation_matrix;
    });
}

} // namespace diffusion {