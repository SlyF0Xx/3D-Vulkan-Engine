#pragma once

#include "edyn/edyn.hpp"
#include <edyn/time/time.hpp>
#include "edyn/comp/tree_resident.hpp"

#include <variant>

namespace edyn {
    //NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(kinematic_tag);
    //NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(static_tag);
    //NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(multi_island_resident, island_entities);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vector3, x, y, z);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(position, x, y, z);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(orientation, x, y, z, w);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(mass, s);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(mass_inv, s);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(inertia, row);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(inertia_inv, row);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(inertia_world_inv, row);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(linvel, x, y, z);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(angvel, x, y, z);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(gravity, x, y, z);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(material, restitution, friction, spin_friction, roll_friction, stiffness, damping, id);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(present_position, x, y, z);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(present_orientation, x, y, z, w);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(box_shape, half_extents);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(shape_index, value);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AABB, min, max);
    /*
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(continuous_contacts_tag);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dynamic_tag);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(procedural_tag);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(kinematic_tag);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(static_tag);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(continuous);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(rigidbody_tag);
    */

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(continuous, types, size);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(island_timestamp, value);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(island_resident, island_entity);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(graph_node, node_index);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(collision_filter, group, mask);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(graph_edge, edge_index);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(contact_manifold, body, separation_threshold, point);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(tree_resident, id, procedural);
    //NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(multi_island_resident, island_entities);

    struct ToJsonFunctions
    {
        nlohmann::json operator()(const box_shape& box)
        {
            nlohmann::json extent;
            nlohmann::to_json(extent, box);
            nlohmann::json j = { {"type", 0}, {"box", box}};
            return j;
        }

        nlohmann::json operator()(const sphere_shape& sphere)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const cylinder_shape& cylinder)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const capsule_shape& capsule)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const polyhedron_shape& polyhedron)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const compound_shape& compound)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const plane_shape& plane)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const mesh_shape& mesh)
        {
            return nlohmann::json();
        }

        nlohmann::json operator()(const paged_mesh_shape& paged_mesh)
        {
            return nlohmann::json();
        }
    };


    inline void to_json(nlohmann::json& j, const shapes_variant_t& shape)
    {
        j = std::visit(ToJsonFunctions{}, shape);
    }

    inline void from_json(const nlohmann::json& j, shapes_variant_t& shape)
    {
        auto const index = j.at("type").get<int>();

        switch (index)
        {
        case 0:
        {
            box_shape box{};
            auto& extent = j.at("box");
            from_json(extent, box);
            shape = box;
            break;
        }
        default:
            throw std::runtime_error{ "" };
        }
    }
}
