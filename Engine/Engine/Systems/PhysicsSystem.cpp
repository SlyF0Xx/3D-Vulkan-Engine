#include "PhysicsSystem.h"

#include <edyn/edyn.hpp>
//#include <edyn/time/time.hpp>
//#include "edyn/comp/tree_resident.hpp"
#include "../Engine.h"
//#include <edyn/time/time.hpp>
//#include "edyn/comp/tree_resident.hpp"
#include "../PhysicsUtils.h"

#include "../Entities/ImportableEntity.h"
#include "../BaseComponents/ScaleComponent.h"
//#include "edyn/collision/contact_point.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace diffusion {

PhysicsSystem::PhysicsSystem(Game& game)
    : m_game(game)
{
    m_game.get_registry().on_update<edyn::position>().connect<&PhysicsSystem::update_phys_component>(*this);
    m_game.get_registry().on_update<TransformComponent>().connect<&PhysicsSystem::update_transform>(*this);
    m_game.get_registry().on_construct<edyn::contact_point>().connect<&PhysicsSystem::on_detect_collision>(*this);
}

void PhysicsSystem::update_transform(::entt::registry& registry, ::entt::entity parent_entity)
{
    if (!m_game.m_paused) {
        return;
    }
    
    auto & transform = registry.get<TransformComponent>(parent_entity);
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(diffusion::calculate_global_world_matrix(registry, transform), scale, rotation, translation, skew, perspective);
    glm::vec3 rotation_euler = glm::eulerAngles(rotation);

    auto * physics_position = registry.try_get<ColliderDefinition>(parent_entity);
    if (!physics_position) {
        return;
    }
    physics_position->transform[0] = translation;
    physics_position->transform[1] = rotation_euler;
    physics_position->transform[2] = scale;
}

void PhysicsSystem::update_phys_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& pos = registry.get<edyn::present_position>(parent_entity);
    auto& orient = registry.get<edyn::orientation>(parent_entity);
    auto& scaleComp = registry.get<ScaleComponent>(parent_entity);

    //auto& bound = registry.get<BoundingComponent>(parent_entity);
    // auto& trans = registry.get<TransformComponent>(parent_entity);
    registry.patch<TransformComponent>(parent_entity, [this, &pos, scale = scaleComp.Scale, &orient](TransformComponent& trans)
    {
        auto ans = glm::translate(glm::mat4(1), { pos.x,pos.y,pos.z }) *
            ([](glm::vec4 q)
                {
                    float qx = q.x;
                    float qy = q.y;
                    float qz = q.z;
                    float qw = -q.w;
                    return glm::mat4{
                        1.0f - 2.0f * qy * qy - 2.0f * qz * qz, 2.0f * qx * qy - 2.0f * qz * qw, 2.0f * qx * qz + 2.0f * qy * qw, 0.0f,
                2.0f * qx * qy + 2.0f * qz * qw, 1.0f - 2.0f * qx * qx - 2.0f * qz * qz, 2.0f * qy * qz - 2.0f * qx * qw, 0.0f,
                2.0f * qx * qz - 2.0f * qy * qw, 2.0f * qy * qz + 2.0f * qx * qw, 1.0f - 2.0f * qx * qx - 2.0f * qy * qy, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
                    };
                })({ orient.x,orient.y,orient.z,orient.w }) *
                    glm::scale(scale);

                //auto quat = glm::quat{orient.x,orient.y, orient.z, orient.w};
                trans.m_world_matrix = ans;
    });
}

void PhysicsSystem::on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& point = registry.get<edyn::contact_point>(parent_entity);

    for (auto& body : point.body) {
        auto* script = registry.try_get<diffusion::ScriptComponentState>(body);
        if (script) {
            auto ref = luabridge::getGlobal(script->m_state, "on_trigger");
            auto ret = ref();
            if (!ret.wasOk()) {
                std::string err = ret.errorMessage();
                std::cerr << err;
            }
        }
    }
}

}