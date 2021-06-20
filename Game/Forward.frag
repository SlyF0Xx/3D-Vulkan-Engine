#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 color; 

layout (location = 0) in vec2 tex_coords; 

layout (set = 2, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 2, binding = 1) uniform sampler2D samplerNormal;

struct LightInfo
{
    vec3 m_position;
};

layout (set = 3, binding = 0) uniform Lights{
	uint size;
	LightInfo lights[10];
} light_buffer;

void main()
{
	vec4 albedo = texture(samplerAlbedo, tex_coords);
	color;
}