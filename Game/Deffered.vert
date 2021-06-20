#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec2 norm_coords;

layout (set = 0, binding = 0) uniform CamMatrix{
	mat4 ViewProjection;
}view_proj_mat;

layout (set = 1, binding = 0) uniform WorldMatrix{
	mat4 World;
}world_mat;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_tex_coords;

void main()
{
	gl_Position = view_proj_mat.ViewProjection * world_mat.World * vec4(position, 1.0f);
	out_tex_coords = tex_coords;
}