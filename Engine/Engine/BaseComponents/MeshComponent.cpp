#include "MeshComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <set>

namespace diffusion {

namespace {

void push_vec_in_sets(const glm::vec3& vec, std::set<float>& x_values, std::set<float>& y_values, std::set<float>& z_values)
{
	x_values.insert(vec.x);
	y_values.insert(vec.y);
	z_values.insert(vec.z);
}

} // unnamed namespace

SubMesh::AABB calculate_bounding_box_in_world_space(entt::registry& registry, const SubMesh & mesh, const TransformComponent& transform)
{
	glm::mat4 global_matrix = calculate_global_world_matrix(registry, transform);

	glm::vec3 min_in_global = global_matrix * glm::vec4(mesh.m_bounding_box.min, 1.0f);
	glm::vec3 max_in_global = global_matrix * glm::vec4(mesh.m_bounding_box.max, 1.0f);

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(diffusion::calculate_global_world_matrix(registry, transform), scale, rotation, translation, skew, perspective);
	glm::vec3 euler = glm::eulerAngles(rotation);

	glm::mat4 rotation_matrix = diffusion::create_matrix(glm::vec3(0), euler, glm::vec3(1));
	glm::vec3 x_positive = rotation_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 y_positive = rotation_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec3 z_positive = rotation_matrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	float length_x = scale.x * (mesh.m_bounding_box.max.x - mesh.m_bounding_box.min.x);
	float length_y = scale.y * (mesh.m_bounding_box.max.y - mesh.m_bounding_box.min.y);
	float length_z = scale.z * (mesh.m_bounding_box.max.z - mesh.m_bounding_box.min.z);

	std::set<float> x_values;
	std::set<float> y_values;
	std::set<float> z_values;

	push_vec_in_sets(min_in_global, x_values, y_values, z_values);

	push_vec_in_sets(min_in_global + x_positive * length_x, x_values, y_values, z_values);
	push_vec_in_sets(min_in_global + y_positive * length_y, x_values, y_values, z_values);
	push_vec_in_sets(min_in_global + z_positive * length_z, x_values, y_values, z_values);

	push_vec_in_sets(min_in_global + x_positive * length_x + y_positive * length_y, x_values, y_values, z_values);
	push_vec_in_sets(min_in_global + x_positive * length_x + z_positive * length_z, x_values, y_values, z_values);
	push_vec_in_sets(min_in_global + y_positive * length_y + z_positive * length_z, x_values, y_values, z_values);

	push_vec_in_sets(max_in_global, x_values, y_values, z_values);

	glm::vec3 min(*x_values.begin(), *y_values.begin(), *z_values.begin());
	glm::vec3 max(*x_values.rbegin(), *y_values.rbegin(), *z_values.rbegin());
	return SubMesh::AABB(min, max);
}

bool is_in_bounding_box(const SubMesh::AABB& bounding_box, const glm::vec3& point)
{
	double x_precision = 1.0;
	double y_precision = 1.0;
	double z_precision = 1.0;

	return bounding_box.min.x - point.x <= x_precision &&
		   bounding_box.min.y - point.y <= y_precision &&
		   bounding_box.min.z - point.z <= z_precision &&
		   point.x - bounding_box.max.x <= x_precision &&
		   point.y - bounding_box.max.y <= y_precision &&
		   point.z - bounding_box.max.z <= z_precision;
}

} // namespace diffusion