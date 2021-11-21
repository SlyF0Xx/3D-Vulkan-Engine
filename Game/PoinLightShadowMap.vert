#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 in_norm;

layout (set = 1, binding = 0) uniform WorldMatrix{
	mat4 World;
}world_mat;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = world_mat.World * vec4(position, 1.0f);
}