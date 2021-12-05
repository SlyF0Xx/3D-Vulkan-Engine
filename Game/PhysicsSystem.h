#pragma once

#include "TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "edyn/edyn.hpp"
#include <edyn/time/time.hpp>

namespace diffusion {

    struct PhysTag
    {
        ::entt::entity m_entity;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysTag, m_entity)
    };

    class PhysicsSystem
    {
    public:
        PhysicsSystem(::entt::registry& registry);

        void tick();

        void add_phys_component(::entt::registry& registry, ::entt::entity parent_entity);
        void on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity);
    
    private:
        ::entt::registry& m_registry;
    };

} // namespace diffusion {