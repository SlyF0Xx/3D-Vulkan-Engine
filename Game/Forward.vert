#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 in_norm;

layout (set = 0, binding = 0) uniform CamMatrix{
	mat4 ViewProjection;
}view_proj_mat;

struct LightInfo
{
    vec3 m_direction;
    mat4 ViewProjection;
};

struct PointLightInfo
{
    vec3 m_position;
};

layout (set = 1, binding = 0) uniform WorldMatrix{
	mat4 World;
}world_mat;

// #pragma pack(push, 1)
layout (set = 3, binding = 0) uniform Lights{
	float light_buffer_size;
	float point_light_buffer_size;
	LightInfo lights[10];
	PointLightInfo point_lights[10];
} light_buffer;
// #pragma pack(pop)

layout(push_constant) uniform PushConsts {
	int is_unlit;
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec3 out_norm;
layout(location = 2) out vec3 out_position;
layout(location = 3) out vec4 out_shadow_coords[10];

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
	gl_Position = view_proj_mat.ViewProjection * world_mat.World * vec4(position, 1.0f);
	out_tex_coords = tex_coords;
	out_norm = in_norm;
	out_position = vec3(world_mat.World * vec4(position, 1.0f));

	if (pushConsts.is_unlit != 1) {
		for (int i = 0; i < light_buffer.light_buffer_size; ++i) {
			out_shadow_coords[i] =  (biasMat * light_buffer.lights[i].ViewProjection * world_mat.World) * vec4(position, 1.0);
		}
	}

}