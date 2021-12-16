#include "KitamoriMovingSystem.h"
#include "BaseComponents/TransformComponent.h"

#include "KitamoriSystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <edyn/edyn.hpp>
#include <entt/entt.hpp>
#include <entt/entity/utility.hpp>


namespace diffusion {

void KitamoriMovingSystem::update_position(glm::vec3 direction)
{
    
    /*auto potential_linked_components = m_registry.view<const KitamoriLinkedTag>();
    potential_linked_components.each([this, &direction](const KitamoriLinkedTag& tag) {
        m_registry.patch<edyn::position>(::entt::to_entity(m_registry, tag), [&direction](edyn::position& pos) {
            pos += {direction.x,direction.y,direction.z};
        });
        //m_registry.get<edyn::position>(::entt::to_entity(m_registry, tag)) += {direction.x,direction.y,direction.z};
        m_registry.get_or_emplace<edyn::dirty>(::entt::to_entity(m_registry, tag)).updated<edyn::position>();
    });*/

    linkedSystem->addTranslation(direction);
    //edyn::update(m_registry);
    //edyn::update(m_registry);
    //update_components();
}

} // namespace diffusion {
