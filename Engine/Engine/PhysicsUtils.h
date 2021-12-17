#pragma once

#include <entt/entity/fwd.hpp>
#include <edyn/edyn.hpp>
#include <glm/glm.hpp> 

#include "BaseComponents/ScaleComponent.h"

enum ECollisionType
{
	Trigger,
	Blocker,
    Static,
    Ignore
};

struct ColliderDefinition {
	ECollisionType CollisionType = ECollisionType::Blocker;
	glm::mat3x3 transform = glm::mat3x3(glm::vec3(0), glm::vec3(0), glm::vec3(1));
	edyn::shapes_variant_t shape = edyn::box_shape{1};
	uint64_t collisionGroup = ~0ULL;
	uint64_t collisionMask = ~0ULL;
    int64_t mass = 0;
};

static void add_collider(entt::entity entity, entt::registry& registry, ColliderDefinition Definition) {
    auto def = edyn::rigidbody_def();
    def.kind = Definition.CollisionType == ECollisionType::Static ? edyn::rigidbody_kind::rb_static : edyn::rigidbody_kind::rb_dynamic;
    def.position = {
        Definition.transform[0].x,
        Definition.transform[0].y,
        Definition.transform[0].z
    };
    
    def.orientation = {
        Definition.transform[1].x,
        Definition.transform[1].y,
        Definition.transform[1].z,
        -1
    };

    registry.emplace<diffusion::ScaleComponent>(entity, Definition.transform[2]);
    
    def.mass = Definition.mass > 0 ? Definition.mass : 1000;
    if (Definition.CollisionType != ECollisionType::Ignore) {
        def.shape = Definition.shape;
    }

    def.update_inertia();

    switch (Definition.CollisionType){
        case ECollisionType::Trigger:
            def.material.reset();
            break;
        case ECollisionType::Blocker:
            def.material->restitution = .75;
            def.material->friction = 100;
            break;
       case ECollisionType::Static:
            def.material->restitution = .75;
            def.material->friction = 100;
            break;
    }

    def.collision_group = Definition.collisionGroup;
    def.collision_mask = Definition.collisionMask;
   
    def.gravity = edyn::vector3_z * -9.8 * Definition.mass;

    def.continuous_contacts = true;
    def.presentation = true;
    edyn::make_rigidbody(entity, registry, def);
}
