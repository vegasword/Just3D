#version 460

#define PI 3.1415926535897932384626433832795
#define MEDIUM_PRECISION_FLT_MAX 65504.0

in vec3 position;
in vec2 uv;
in vec4 baseColorFactor;
in float metallicFactor;
in float roughnessFactor;
in vec3 cameraPosition;
in vec3 view;
in vec3 light;
in vec3 halfVector;
in mat3 tbn;

layout (binding = 0) uniform sampler2D baseColorMap;
layout (binding = 1) uniform sampler2D metallicRoughnessMap;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 3) uniform sampler2D ambientOcclusionMap;

out vec4 o_color;

float GGXNormalDistribution(float normalDotHalf, float roughness, vec3 normal, vec3 halfVector)
{
  vec3 normalCrossHalf = cross(normal, halfVector);
  float cosScaled = normalDotHalf * roughness;
  float distributionTerm = roughness / dot(normalCrossHalf, normalCrossHalf) + cosScaled * cosScaled;
  return min(distributionTerm * distributionTerm / PI, MEDIUM_PRECISION_FLT_MAX);
}

float SmithGGXCorrelatedVisibility(float normalDotView, float normalDotLight, float roughness)
{
  float roughnessComplement = 1.0 - roughness;
  float viewGGX = normalDotLight * (normalDotView * roughnessComplement + roughness);
  float lightGGX = normalDotView * (normalDotLight * roughnessComplement + roughness);
  return 0.5 / (viewGGX + lightGGX);
}

vec3 SchlickFresnel(float viewDotHalf, vec3 normalIncidenceReflectance)
{
  return normalIncidenceReflectance + (vec3(1.0) - normalIncidenceReflectance) * pow(1.0 - viewDotHalf, 5.0);;
}

void main()
{
  vec3 baseColor = texture(baseColorMap, uv).rgb * baseColorFactor.rgb;
  vec3 metallicRoughness = normalize(texture(metallicRoughnessMap, uv).rgb);
  vec3 normal = normalize(tbn * (texture(normalMap, uv).rgb * 2.0 - 1.0));
  vec3 ambientOcclusion = normalize(texture(ambientOcclusionMap, uv).rgb);

  float metallic = metallicRoughness.b * metallicFactor;
  float roughness = metallicRoughness.g * roughnessFactor;
  roughness *= roughness; // Remapped to a perceptually linear range  
  float reflectance = 0.04; // For now everything will look like plastic
  vec3 normalIncidenceSpecularReflectance  = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic;
  
  float viewDotHalf = dot(view, halfVector);
  float normalDotHalf = dot(normal, halfVector);
  float normalDotView = dot(normal, view);
  float normalDotLight = dot(normal, light);
  float lightDotHalf = dot(light, halfVector);
  
  float distribution = GGXNormalDistribution(normalDotHalf, roughness, normal, halfVector);
  float visibility = SmithGGXCorrelatedVisibility(normalDotView, normalDotHalf, roughness);
  vec3 fresnel = SchlickFresnel(viewDotHalf, normalIncidenceSpecularReflectance);
  // TODO: https://google.github.io/filament/Filament.html#toc5.3
  // vec3 energyCompensation = 1.0 + normalIndicidenceSpecularReflectance * (1.0 / dfg.y - 1.0);
  vec3 specularBRDF = distribution * visibility * fresnel; // * energyCompensation;
  vec3 diffuseBRDF = (1.0 - metallic) * baseColor / PI;
  
  o_color = vec4(diffuseBRDF, 1.0);
}
