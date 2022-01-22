#include "ScriptComponent.h"

#include "CameraComponent.h"
#include "TransformComponent.h"
#include "TagComponent.h"
#include "VulkanComponents/ImportableVulkanMeshComponents.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "../Entities/PlaneEntity.h"
#include "../Entities/DebugCube.h"
#include "../Entities/CubeEntity.h"
#include <glm/gtx/matrix_decompose.hpp>

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
        /*.beginClass<LuaVector>("Vector3")
            .addProperty("x", &LuaVector::x)
            .addProperty("y", &LuaVector::y)
            .addProperty("z", &LuaVector::y)
        .endClass()*/
        .beginClass<entt::entity>("entity")
        .endClass()
        .beginNamespace("log")
        .addFunction("print", [](const std::string& s) {
            std::cout << s << std::endl;
        })
        .addFunction("clear", []() {
#if defined _WIN32
            system("cls");
            //clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
            system("clear");
            //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
            system("clear");
#endif
        })
        .endNamespace()
        .beginNamespace("primitives")
        .addFunction("cube", [&registry](float x, float y, float z) {
            auto cube_entity = diffusion::create_cube_entity(registry, {x, y, z}, {0, 0, 0}, {1, 1, 1});
            add_default_unlit_material_component(registry, cube_entity);
            return cube_entity;
        })
        .addFunction("plane", [&registry](float x, float y, float z) {
            return diffusion::create_plane_entity_lit(registry, {x, y, z});
        })
        .addFunction("debug_sphere", [&registry](float x, float y, float z) {
            return diffusion::create_debug_sphere_entity(registry, {x, y, z});
        })
        .addFunction("debug_cube", [&registry](float x, float y, float z) {
            return diffusion::create_debug_cube_entity(registry, {x, y, z});
        })
        .endNamespace()
        .beginNamespace("camera")
        .addFunction("look_at", [&registry](entt::entity& entity, float x, float y, float z) {
            if (registry.view<CameraComponent>().contains(entity)) {
                registry.patch<diffusion::TransformComponent>(entity, [&](diffusion::TransformComponent& transform) {
                    glm::vec3 scale;
                    glm::quat rotation;
                    glm::vec3 translation;
                    glm::vec3 skew;
                    glm::vec4 perspective;
                    glm::decompose(transform.m_world_matrix, scale, rotation, translation, skew, perspective);

                    transform.m_world_matrix 
                        = diffusion::create_matrix_by_location_target(translation, glm::vec3(x, y, z));
                });
            }
        })
        .endNamespace()
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
        })
        .addFunction("destroy_entity", [&registry](entt::entity& entity) {
            registry.destroy(entity);
        })
        .addFunction("get_position", [&registry](entt::entity& entity) {
            if (registry.valid(entity) && registry.view<TransformComponent>().contains(entity)) {
                auto& transform = registry.get<TransformComponent>(entity);
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(
                    transform.m_world_matrix, 
                    scale, rotation, translation, skew, perspective
                );

                return std::tuple<float, float, float, bool>(
                    translation.x, 
                    translation.y, 
                    translation.z,
                    true
                );
            }
            return std::tuple<float, float, float, bool>(
                -FLT_MAX,
                -FLT_MAX,
                -FLT_MAX,
                false
            );;
        })
        .addFunction("get_rotation", [&registry](entt::entity& entity) {
            if (registry.valid(entity) && registry.view<TransformComponent>().contains(entity)) {
                auto& transform = registry.get<TransformComponent>(entity);
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(
                    transform.m_world_matrix,
                    scale, rotation, translation, skew, perspective
                );
                rotation = glm::conjugate(rotation);
                glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

                return std::tuple<float, float, float, bool>(
                    euler.x,
                    euler.y,
                    euler.z,
                    true
                    );
            }
            return std::tuple<float, float, float, bool>(
                -FLT_MAX,
                -FLT_MAX,
                -FLT_MAX,
                false
                );;
        })
        .addFunction("get_scale", [&registry](entt::entity& entity) {
            if (registry.valid(entity) && registry.view<TransformComponent>().contains(entity)) {
                auto& transform = registry.get<TransformComponent>(entity);
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(
                    transform.m_world_matrix,
                    scale, rotation, translation, skew, perspective
                );

                return std::tuple<float, float, float, bool>(
                    scale.x,
                    scale.y,
                    scale.z,
                    true
                    );
            }
            return std::tuple<float, float, float, bool>(
                -FLT_MAX,
                -FLT_MAX,
                -FLT_MAX,
                false
                );;
        });
    return state;
}

} // namespace diffusion 