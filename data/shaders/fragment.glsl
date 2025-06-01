#version 460

#define REFLECTANCE 0.04
#define PI 3.1415926535897932384626433832795
#define INV_PI 0.31830988618379067153776752674503
    
const float lightIntensity = 5; // For now unitless
const vec3 light = vec3(0.0, 1.0, -1.0); // Directional for now
const vec3 normalIncidenceReflectance = vec3(REFLECTANCE);

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 cameraPosition;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 baseColor;
layout (location = 4) in float metallicFactor;
layout (location = 5) in float roughnessFactor;
layout (location = 6) in mat3 tbn;

layout (binding = 0) uniform sampler2D baseColorMap;
layout (binding = 1) uniform sampler2D metallicRoughnessMap;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 3) uniform sampler2D ambientOcclusionMap;

out vec4 o_color;

float GGXNormalDistribution(float normalDotHalf, float roughness, const vec3 normal, const vec3 halfVector)
{
  float roughnessSq = roughness * roughness;
  float heaviside = (normalDotHalf * normalDotHalf) * (roughnessSq - 1.0) + 1.0;
  return roughnessSq / (PI * heaviside * heaviside);  
}

float SmithGGXCorrelatedVisibility(float normalDotView, float normalDotLight, float roughness)
{
  float viewGGX = normalDotLight * (normalDotView * (1.0 - roughness) + roughness);
  float lightGGX = normalDotView * (normalDotLight * (1.0 - roughness) + roughness);
  float GGX = viewGGX + lightGGX;
  return GGX > 0 ? 0.5 / GGX : 0;
}

vec3 SchlickFresnelVector(float viewDotHalf, vec3 normalIncidenceReflectance, vec3 grazingReflectance)
{
  return normalIncidenceReflectance + (grazingReflectance - normalIncidenceReflectance) * pow(1.0 - viewDotHalf, 5.0);
}

void main()
{  
  vec3 view =  normalize(cameraPosition - position);
  vec3 halfVector = normalize(view + light);
  
  vec3 baseColor = texture(baseColorMap, uv).rgb * baseColor.rgb;
  vec3 metallicRoughness = texture(metallicRoughnessMap, uv).rgb;
  vec3 normal = normalize(tbn * (texture(normalMap, uv).rgb * 2.0 - 1.0));
  float ambientOcclusion = texture(ambientOcclusionMap, uv).r;

  float metallic = metallicRoughness.b * metallicFactor;
  float alphaRoughness = metallicRoughness.g * roughnessFactor;
  alphaRoughness *= alphaRoughness; // Results in more perceptually linear changes in the roughness

  float normalDotLight = clamp(dot(normal, light), 0.0, 1.0);
  float normalDotView = clamp(dot(normal, view), 0.0, 1.0);
  float normalDotHalf = clamp(dot(normal, halfVector), 0.0, 1.0);
  float lightDotHalf = clamp(dot(light, halfVector), 0.0, 1.0);
  float viewDotHalf = abs(dot(view, halfVector));
  
  float distribution = GGXNormalDistribution(normalDotHalf, alphaRoughness, normal, halfVector);
  float visibility = SmithGGXCorrelatedVisibility(normalDotView, normalDotLight, alphaRoughness);
  vec3 specularMetal = normalDotLight * vec3(distribution * visibility);
  
  vec3 dielectricFresnel = SchlickFresnelVector(viewDotHalf, normalIncidenceReflectance, vec3(1.0));
  vec3 metalFresnel = SchlickFresnelVector(viewDotHalf, baseColor.rgb, vec3(1.0));
  
  vec3 metalBRDF = metalFresnel * specularMetal;
  vec3 dielectricBRDF = mix(dielectricFresnel, metalFresnel, metallic);
  vec3 diffuseBRDF = (1.0 - dielectricBRDF) * baseColor.rgb * INV_PI;
  vec3 specularBRDF = dielectricBRDF * specularMetal;

  o_color = vec4((diffuseBRDF + specularBRDF) * normalDotLight * lightIntensity * ambientOcclusion, 1.0);
}
