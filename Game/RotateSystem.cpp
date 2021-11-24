#include "RotateSystem.h"

#include "Entities/ImportableEntity.h"

namespace diffusion {

RotateSystem::RotateSystem(::entt::registry& registry)
    : m_registry(registry)
{
}

void RotateSystem::tick()
{
    auto& entity = m_registry.ctx<RotateTag>().m_entity;
    m_registry.patch<TransformComponent>(entity, [this] (TransformComponent & transform) {
        rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
        transform.m_world_matrix = transform.m_world_matrix * rotation_matrix;
    });
}

} // namespace diffusion {