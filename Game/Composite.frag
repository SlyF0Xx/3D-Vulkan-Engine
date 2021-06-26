#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D samplerAlbedo;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform usampler2D samplerDepth;
layout (binding = 3) uniform sampler2DArray samplerShadows;

layout (location = 0) out vec4 color; 

layout (location = 0) in vec2 inUV; 

struct LightInfo
{
    vec3 m_direction;
};

// #pragma pack(push, 1000)
layout (set = 1, binding = 0) uniform Lights{
	LightInfo lights[100];
} light_buffer;
// #pragma pack(pop)

layout (set = 1, binding = 1) uniform LightsCount{
	uint light_buffer_size;
} lights_count;

void main()
{
	vec4 albedo = texture(samplerAlbedo, inUV);
	vec4 depth = texture(samplerDepth, inUV);

	if (depth.x < 0.3) { // is unlit
		color = albedo;
	} else {
		vec3 normal = (texture(samplerNormal, inUV).rgb - vec3(0.5f)) * 2;

		vec3 fragcolor = vec3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < lights_count.light_buffer_size; ++i) {
			// Light to fragment
			vec3 L = normalize(light_buffer.lights[i].m_direction);
			L = vec3(L.x, -L.z, L.y);

			// Diffuse part
			vec3 N = normalize(normal);
			float NdotL = max(0.0, dot(N, L));
			vec3 diff = vec3(1.0f, 1.0f, 1.0f) * albedo.rgb * NdotL;
			// ubo.lights[i].color

			fragcolor += diff;
		}

		color = vec4(fragcolor, 1.0f);
	}
}