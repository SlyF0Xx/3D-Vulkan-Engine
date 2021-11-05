#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 albedo; 
layout (location = 1) out vec3 normal; 

layout (location = 0) in vec2 tex_coords; 
layout (location = 1) in vec3 in_norm; 

layout (set = 2, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 2, binding = 1) uniform sampler2D samplerNormal;

void main()
{
	albedo = texture(samplerAlbedo, tex_coords);

	vec3 normalized = normalize(texture(samplerNormal, tex_coords).rgb);
	vec3 normalVec = normalize(in_norm);
	normal = normalize(normalized * normalVec) / 2 + vec3(0.5f);
}