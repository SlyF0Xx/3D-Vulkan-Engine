#pragma once

#include "System.h"
#include "TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>

namespace diffusion {

namespace entt {

struct RotateTag
{
    ::entt::entity m_entity;
};

}

class RotateSystem /* : public System */
{
public:
    RotateSystem(::entt::registry& registry);

    /*
    void components_callback(const std::vector<std::reference_wrapper<Component>>& components) override {
        rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
        for (auto& component : components) {
            auto & comp = dynamic_cast<TransformComponent&>(component.get());
            comp.UpdateWorldMatrix(comp.get_world_matrix() * rotation_matrix);
        }
    }
    */

    void tick();

private:
    ::entt::registry& m_registry;
    glm::mat4 rotation_matrix{ 1 };
    glm::vec3 RotationZ{ 0, 0, 1.0 };
};

} // namespace diffusion {