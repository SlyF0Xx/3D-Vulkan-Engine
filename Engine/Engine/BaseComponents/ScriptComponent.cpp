#include "ScriptComponent.h"

#include "TransformComponent.h"
#include "TagComponent.h"
#include "VulkanComponents/ImportableVulkanMeshComponents.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace diffusion {

entt::entity get_entity_by_name(entt::registry& registry, const char* name)
{
    std::string_view name_view(name);
    auto view = registry.view<TagComponent>();
    for (auto& entity : view) {
        auto& tag_component = registry.get<TagComponent>(entity);
        if (tag_component.m_Tag == name_view) {
            return entity;
        }
    }
    return entt::entity();
}

lua_State* create_lua_state(entt::registry& registry)
{
    auto state = luaL_newstate();
    luaL_openlibs(state);
    lua_settop(state, 0);

    getGlobalNamespace(state)
        .beginClass<entt::entity>("entity")
        .endClass()
        /*
        .beginClass<TransformComponent>("transform")
        .endClass()
        .addFunction("get_transform_component", [this](entt::entity entity) {
            return registry.get<TransformComponent>(entity);
        })
        .addFunction("global_translate", [this](TransformComponent& transform, float x, float y, float z) {
            registry.patch<TransformComponent>(entt::to_entity(registry, transform),
                transform.m_world_matrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
            });
        })
        .addFunction("local_translate", [this](TransformComponent& transform, float x, float y, float z) {
            registry.patch<TransformComponent>(entt::to_entity(registry, transform),
                [x, y, z](auto& transform) {
                transform.m_world_matrix = glm::translate(transform.m_world_matrix, glm::vec3(x, y, z));
            });
        })
        */
        .addFunction("global_translate", [&registry](entt::entity& entity, float x, float y, float z) {
            registry.patch<TransformComponent>(entity, [x, y, z](auto& transform) {
                transform.m_world_matrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
            });
        })
        .addFunction("local_translate", [&registry](entt::entity& entity, float x, float y, float z) {
            registry.patch<TransformComponent>(entity, [x, y, z](auto& transform) {
                transform.m_world_matrix = glm::translate(transform.m_world_matrix, glm::vec3(x, y, z));
            });
        })
        .addFunction("local_rotate", [&registry](entt::entity& entity, float x, float y, float z) {
            registry.patch<TransformComponent>(entity, [x, y, z](auto& transform) {
            glm::vec3 RotationX(1.0, 0, 0);
                transform.m_world_matrix = glm::rotate(transform.m_world_matrix, x, RotationX);

                glm::vec3 RotationY(0, 1.0, 0);
                transform.m_world_matrix = glm::rotate(transform.m_world_matrix, y, RotationY);

                glm::vec3 RotationZ(0, 0, 1.0);
                transform.m_world_matrix = glm::rotate(transform.m_world_matrix, z, RotationZ);
            });
        })
        .addFunction("local_scale", [&registry](entt::entity& entity, float x, float y, float z) {
            registry.patch<TransformComponent>(entity, [x, y, z](auto& transform) {
                glm::mat4 scale_matrix = glm::scale(glm::vec3(x, y, z));
                transform.m_world_matrix *= scale_matrix;
            });
        })
        .addFunction("spawn_entity", [&registry]() {
            return registry.create();
        })
        .addFunction("change_name", [&registry](entt::entity& entity, const char* name) {
            registry.emplace_or_replace<TagComponent>(entity, name);
        })
        .addFunction("add_transform", [&registry](entt::entity& entity) {
            registry.emplace<TransformComponent>(entity, glm::mat4(1));
        })
        .addFunction("import_mesh", [&registry](entt::entity& entity, const char* path) {
            import_mesh(std::filesystem::path(path), registry, entity);
        })
        .addFunction("get_entity_by_name", [&registry](const char* name) {
            return get_entity_by_name(registry, name);
        });
    return state;
}

} // namespace diffusion 