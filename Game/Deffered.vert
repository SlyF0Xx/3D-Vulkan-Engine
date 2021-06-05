#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 in_color;

layout (binding = 0) uniform CamMatrixes{
	mat4 WorldViewProjection;
}Mat;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec4 color;

void main()
{
	gl_Position = Mat.WorldViewProjection * vec4(position, 1.0f);
	color = in_color;
}