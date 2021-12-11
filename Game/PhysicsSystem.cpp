#include "PhysicsSystem.h"

#include "Entities/ImportableEntity.h"
#include "KitamoriSystem.h"
//#include "edyn/collision/contact_point.hpp"

diffusion::PhysicsSystem::PhysicsSystem(::entt::registry& registry) 
	: m_registry(registry)
{
    m_registry.on_update<edyn::position>().connect<&PhysicsSystem::add_phys_component>(*this);
    m_registry.on_construct<edyn::contact_point>().connect<&PhysicsSystem::on_detect_collision>(*this);
}

void diffusion::PhysicsSystem::tick()
{
    
  /*  auto view2 = m_registry.view<edyn::position, TransformComponent>();
    for (auto& entity : view2)
    {
        edyn::position& pos = m_registry.get<edyn::position>(entity);
        TransformComponent& tc = m_registry.get<TransformComponent>(entity);

        tc.m_world_matrix = create_matrix(glm::vec3(pos.x,pos.y,pos.z),glm::vec3(0), glm::vec3(1));
    }*/
    
    //edyn::linvel vel = edyn::vector3_z * 3;
    /*auto view = m_registry.view<edyn::position, edyn::dynamic_tag>();
    for (auto& entity : view)
    {
        //m_registry.get_or_emplace<edyn::dirty>(entity).updated<edyn::position>();
      //m_registry.emplace<edyn::dirty>(entity).updated<edyn::position>();
        m_registry.patch<edyn::position>(entity,[this](edyn::position& pos)
        {
            pos += edyn::vector3_z * -.5;
        });
        
      edyn::position& pos = m_registry.get<edyn::position>(entity);*/
      //pos ;
      /*edyn::linvel& vel = m_registry.get<edyn::linvel>(entity);
      vel *= -1;*/
        //
        //m_registry.get_or_emplace<edyn::dirty>(entity).updated<edyn::position, edyn::linvel>();
      //vel += edyn::vector3_z * -2;
      //edyn::refresh<edyn::position>(m_registry,entity);
      //m_registry.get_or_emplace<edyn::dirty>(entity).updated<edyn::present_position>();
      // m_registry.emplace<edyn::dirty>(entity);
       /* m_registry.patch<TransformComponent>(entity,[this, &pos](TransformComponent& trans)
      {
          trans.m_world_matrix = create_matrix(glm::vec3(pos.x,pos.y,pos.z),glm::vec3(0), glm::vec3(1));
      });*/
        
      /*TransformComponent& trans = m_registry.get<TransformComponent>(entity);
      trans.m_world_matrix = create_matrix(glm::vec3(pos.x,pos.y,pos.z),glm::vec3(0), glm::vec3(1));*/
    //}
    //edyn::update
   //edyn::update(m_registry);
    /*
    view.each([] (edyn::linvel &pos, edyn::position&pos1) {
      pos += {0,0,10};
      
      pos1 = pos;
    });
   /* auto view = m_registry.view<const edyn::position, const edyn::present_position>();
    view.each([this](auto ent, const auto& pos, const auto& curpos) {
        auto& entity = m_registry.ctx<PhysTag>().m_entity;
        m_registry.patch<TransformComponent>(entity, [this, &pos](TransformComponent& transform) {
            transform.m_world_matrix = create_matrix(glm::vec3{ pos.x, pos.y, pos.z }, glm::vec3{0,0,0}, glm::vec3{ 1,1,1 });
                /*rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
            transform.m_world_matrix = transform.m_world_matrix * rotation_matrix;
            });
        });*/
}

void diffusion::PhysicsSystem::add_phys_component(::entt::registry& registry, ::entt::entity parent_entity)
{
    auto& pos = registry.get<edyn::position>(parent_entity);
   // auto& trans = registry.get<TransformComponent>(parent_entity);
    registry.patch<TransformComponent>(parent_entity,[this, &pos](TransformComponent& trans)
    {
        trans.m_world_matrix = create_matrix(glm::vec3(pos.x,pos.y,pos.z),glm::vec3(0), glm::vec3(1));
    });
}

void diffusion::PhysicsSystem::on_detect_collision(::entt::registry& registry, ::entt::entity parent_entity)
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
    /*
    if (registry.try_get<KitamoriLinkedTag>(point.body[0]) || registry.try_get<KitamoriLinkedTag>(point.body[1]))
    {
        registry.emplace_or_replace<KitamoriLinkedTag>(point.body[0]);
        registry.emplace_or_replace<KitamoriLinkedTag>(point.body[1]);
    }
    */
    /*auto [con_ent, constraint] = edyn::make_constraint<edyn::distance_constraint>(registry,point.body[1],point.body[0]);

    
    
    constraint.pivot[0] = edyn::vector3_zero;
    constraint.pivot[1] = edyn::vector3_zero;
    constraint.distance = 0;*/
    //edyn::update(registry);
}
