#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_texture_cube_map_array: require

layout (location = 0) out vec4 color; 

layout (location = 0) in vec2 tex_coords;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec3 position;
layout (location = 3) in vec4 shadow_coords[10];

layout (set = 2, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 2, binding = 1) uniform sampler2D samplerNormal;

struct LightInfo
{
    vec3 m_direction;
    mat4 ViewProjection;
};

struct PointLightInfo
{
    vec3 m_position;
};

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

layout (set = 4, binding = 0) uniform sampler2DArrayShadow ShadowMaps;
layout (set = 4, binding = 1) uniform samplerCubeArrayShadow PointLightShadowMaps;

void main()
{
	vec4 albedo = texture(samplerAlbedo, tex_coords);

	if (pushConsts.is_unlit == 1) {
		color = albedo;
	} else {
		vec3 normal = normalize(texture(samplerNormal, tex_coords).rgb);
		vec3 normalVec = normalize(in_norm);
		normal = normalVec * normal;

		// Diffuse part
		vec3 N = normalize(normal);

		vec3 fragcolor = vec3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < light_buffer.light_buffer_size; ++i) {
			// Light to fragment
			vec3 L = normalize(light_buffer.lights[i].m_direction);

			float NdotL = max(0.0, dot(N, -L));
			vec3 diffuse = vec3(1.0f, 1.0f, 1.0f) * albedo.rgb * NdotL;
			// ubo.lights[i].color

			vec3 shadow = vec3(1.0f);
			vec4 normalized_shadow_coords = shadow_coords[i] / shadow_coords[i].w;

			float dist = texture(ShadowMaps, vec4(normalized_shadow_coords.xy, i, 0));
			if (normalized_shadow_coords.z - dist > 0.01f) 
			{
				shadow = vec3(0.1);
			}

			fragcolor += diffuse * shadow;
		}
		for (int i = 0; i < light_buffer.point_light_buffer_size; ++i) {
			// Light to fragment
			vec3 lights = position - light_buffer.point_lights[i].m_position;
			vec3 L = normalize(lights);

			float NdotL = max(0.0, dot(N, -L));
			vec3 diffuse = vec3(1.0f, 1.0f, 1.0f) * albedo.rgb * NdotL;

			vec3 shadow = vec3(1.0f);

			float distance = ((1.0f / ( length(lights) / 2.0f ) ) - (1.0f / 0.1f)) / ((1.0f / 100.0f) - (1.0f / 0.1f));
			//float distance = ((2.0f / length(lights) ) - 10.0f) / -9.99f;

			// Don't say anyting
			//lights.z = -lights.z;
			//if (lights.y < 0)

			//float dist = texture(PointLightShadowMaps, vec4(-lights, i), 0);
			float dist = texture(PointLightShadowMaps, vec4(lights, i), 0);

			if (distance - dist > 0.01f) 
			{
				shadow = vec3(0.1);
			}

			fragcolor += diffuse * shadow;
		}

		color = vec4(fragcolor, 1.0f);	
	}
}