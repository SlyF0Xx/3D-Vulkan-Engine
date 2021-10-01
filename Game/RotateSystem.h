#pragma once

#include "System.h"
#include "TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

class RotateSystem : public System
{
public:
    void components_callback(const std::vector<std::reference_wrapper<Component>>& components) override {
        rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
        for (auto& component : components) {
            auto comp = dynamic_cast<TransformComponent&>(component.get());
            comp.UpdateWorldMatrix(comp.get_world_matrix() * rotation_matrix);
        }
    }

private:
    glm::mat4 rotation_matrix{ 1 };
    glm::vec3 RotationZ{ 0, 0, 1.0 };
};

} // namespace diffusion {