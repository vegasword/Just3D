#version 460

in vec3 vertexPosition;
in vec2 vertexUV;
in vec4 baseColorFactor;
in float metallicFactor;
in float roughnessFactor;
in vec3 cameraPosition;
in mat3 tbn;

layout (binding = 0) uniform sampler2D baseColor;
layout (binding = 1) uniform sampler2D metallicRoughness;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 3) uniform sampler2D ambientOcclusion;

out vec4 o_color;

void main()
{
  vec3 normal = normalize(tbn * (texture(normalMap, vertexUV).xyz * 2.0 - 1.0));    
  o_color = vec4(normal, 1.0);
  // TODO: o_color = mix(dielectric_brdf, metal_brdf, metallic);
}
