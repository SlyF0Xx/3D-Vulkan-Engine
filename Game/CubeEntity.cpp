#include "CubeEntity.h"

#include "BoundingComponent.h"
#include "TransformComponent.h"
#include "PossessedComponent.h"
#include "VulkanCameraComponent.h"

namespace diffusion {

CubeEntity::CubeEntity(Game& game, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : PrimitiveEntity(
        game,
        {
          PrimitiveColoredVertex{ 0.5f, -0.5f,  0.5f, {0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5f, -0.5f, -0.5f, {0.0f, 0.0f}},
          PrimitiveColoredVertex{-0.5f, -0.5f,  0.5f, {1.0f, 1.0f}},
          PrimitiveColoredVertex{-0.5f, -0.5f, -0.5f, {1.0f, 0.0f}},

          PrimitiveColoredVertex{ 0.5f,  0.5f,  0.5f, {0.0f, 1.0f}},
          PrimitiveColoredVertex{ 0.5f,  0.5f, -0.5f, {0.0f, 0.0f}},
          PrimitiveColoredVertex{-0.5f,  0.5f,  0.5f, {1.0f, 1.0f}},
          PrimitiveColoredVertex{-0.5f,  0.5f, -0.5f, {1.0f, 0.0f}}
        },
        { 
          // forward
          2, 1, 0,
          2, 3, 1,

          // right
          5, 4, 0,
          1, 5, 0,

          // left
          2, 6, 7,
          2, 7, 3,

          // backward
          6, 5, 4,
          6, 7, 5,

          // down
          7, 5, 1,
          3, 7, 1,

          // top
          0, 4, 6,
          0, 6, 2
        },
        translation,
        rotation,
        scale
    )
{
    add_component(ComponentGuard(std::make_unique<BoundingComponent>(glm::vec3(-0.5f, 0.5f, 0.6f), 0.25f, std::vector<Component::Tag>{}, this)));
}

CubePossesedEntity::CubePossesedEntity(Game& game, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : CubeEntity(game, translation, rotation, scale)
{
    add_component(ComponentGuard(std::make_unique<PossessedComponent>(std::vector<Component::Tag>{}, this)));
    add_component(ComponentGuard(std::make_unique<VulkanCameraComponent>(game, std::vector<Component::Tag>{ CameraComponent::s_main_camera_component_tag }, this)));
}

} // namespace diffusion {