#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D samplerAlbedo;

layout (location = 0) out vec4 color; 

layout (location = 0) in vec2 inUV; 

struct LightInfo
{
    vec3 m_position;
};

layout (set = 1, binding = 0) uniform Lights{
	uint size;
	LightInfo lights[10];
} light_buffer;

void main()
{
	vec4 albedo = texture(samplerAlbedo, inUV);
	color = albedo;
}