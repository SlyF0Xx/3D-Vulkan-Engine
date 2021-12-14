#include "PhysicsSystem.h"

#include "Entities/ImportableEntity.h"
#include "KitamoriSystem.h"
#include "BaseComponents/ScaleComponent.h"
//#include "edyn/collision/contact_point.hpp"

diffusion::PhysicsSystem::PhysicsSystem(::entt::registry& registry) 
	: m_registry(registry)
{
    m_registry.on_update<edyn::position>().connect<&PhysicsSystem::update_phys_component>(*this);
    m_registry.on_construct<edyn::contact_point>().connect<&PhysicsSystem::on_detect_collision>(*this);
}

void diffusion::PhysicsSystem::tick()
{
    auto saveTranslation = translation;
    auto potential_linked_components = m_registry.view<const KitamoriLinkedTag>();
    potential_linked_components.each([this](const KitamoriLinkedTag& tag) {
        m_registry.patch<edyn::position>(::entt::to_entity(m_registry, tag), [this](edyn::position& pos) {
            pos += {translation.x,translation.y,translation.z};
        });
        //m_registry.get<edyn::position>(::entt::to_entity(m_registry, tag)) += {direction.x,direction.y,direction.z};
        m_registry.get_or_emplace<edyn::dirty>(::entt::to_entity(m_registry, tag)).updated<edyn::position>();
    });
    //edyn::update(m_registry);

    
    translation -= saveTranslation;

}

void diffusion::PhysicsSystem::addTranslation(glm::vec3 translation)
{
    this->translation += translation;// + translation + translation;
}

void diffusion::PhysicsSystem::update_phys_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& pos = registry.get<edyn::present_position>(parent_entity);
    auto& orient = registry.get<edyn::orientation>(parent_entity);
    auto& scaleComp = registry.get<ScaleComponent>(parent_entity);
    
    //auto& bound = registry.get<BoundingComponent>(parent_entity);
   // auto& trans = registry.get<TransformComponent>(parent_entity);
    registry.patch<TransformComponent>(parent_entity,[this, &pos, scale = scaleComp.Scale, &orient](TransformComponent& trans)
    {
        auto ans = glm::translate(glm::mat4(1), {pos.x,pos.y,pos.z}) *
        ([](glm::vec4 q)
        {
            float qx = q.x;
            float qy = q.y;
            float qz = q.z;
            float qw = -q.w;
            return glm::mat4{
                1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f,
        2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f,
        2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
            };
        })({orient.x,orient.y,orient.z,orient.w}) *
        glm::scale(scale);
        
        //auto quat = glm::quat{orient.x,orient.y, orient.z, orient.w};
        trans.m_world_matrix = ans;
    });
}

void diffusion::PhysicsSystem::on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& point = registry.get<edyn::contact_point>(parent_entity);
    
    if (registry.try_get<KitamoriLinkedTag>(point.body[0]) || registry.try_get<KitamoriLinkedTag>(point.body[1]))
    {
        registry.emplace_or_replace<KitamoriLinkedTag>(point.body[0]);
        registry.emplace_or_replace<KitamoriLinkedTag>(point.body[1]);
    }
    
    /*auto [con_ent, constraint] = edyn::make_constraint<edyn::distance_constraint>(registry,point.body[1],point.body[0]);

    
    
    constraint.pivot[0] = edyn::vector3_zero;
    constraint.pivot[1] = edyn::vector3_zero;
    constraint.distance = 0;*/
    //edyn::update(registry);
}
