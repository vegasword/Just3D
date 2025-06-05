#version 460

#define REFLECTANCE 0.04
#define PI 3.1415926535897932384626433832795
#define INV_PI 0.31830988618379067153776752674503

struct PunctualLight
{
  vec4 position;
  vec4 direction;
  vec3 color;
  float intensity;
  float inverseFalloffRadius;
  float isSpot;
  float cosOuterAngle;
  float spotScale;
};

layout (std430, binding = 5) readonly buffer DirectLights
{
  uint punctalsCount;
  PunctualLight punctals[];
} directLights;

const vec3 normalIncidenceReflectance = vec3(REFLECTANCE); // Will be variant for differents materials in future implementation

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

layout (location = 0) out vec4 o_color;

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

vec3 SchlickFresnelVector(float lightAngle, vec3 normalIncidenceReflectance, vec3 grazingReflectance)
{
  return normalIncidenceReflectance + (grazingReflectance - normalIncidenceReflectance) * pow(1.0 - lightAngle, 5.0);
}

float SquareFalloffAttenuation(vec3 positionToLight, float lightInverseFalloffRadius)
{
  float distanceSquare = dot(positionToLight, positionToLight);
  float factor = distanceSquare * lightInverseFalloffRadius * lightInverseFalloffRadius;
  float smoothFactor = max(1.0 - factor * factor, 0.0);
  return (smoothFactor * smoothFactor) / max(distanceSquare, 0.0001);
}

float SpotAngleAttenuation(PunctualLight light, vec3 positionToLight)
{
  float spotOffset = -light.cosOuterAngle * light.spotScale;
  float luminousIntensity = dot(normalize(-light.direction.xyz), normalize(positionToLight));
  float attenuation = clamp(luminousIntensity * light.spotScale + spotOffset, 0.0, 1.0);
  return attenuation * attenuation;
}

void main()
{  
  vec3 view = normalize(cameraPosition - position);
  
  vec3 baseColor = texture(baseColorMap, uv).rgb * baseColor.rgb;
  vec3 metallicRoughness = texture(metallicRoughnessMap, uv).rgb;
  vec3 normal = normalize(tbn * (texture(normalMap, uv).rgb * 2.0 - 1.0));
  float ambientOcclusion = texture(ambientOcclusionMap, uv).r;

  float metallic = metallicRoughness.b * metallicFactor;
  float alphaRoughness = metallicRoughness.g * roughnessFactor;
  alphaRoughness *= alphaRoughness; // Results in more perceptually linear changes in the roughness

  float normalDotView = clamp(dot(normal, view), 0.0, 1.0);
  
  for (int i = 0; i < directLights.punctalsCount; i++)
  {
    PunctualLight light = directLights.punctals[i];
    
    vec3 lightPosition = light.position.xyz;
    vec3 positionToLight = lightPosition - position;
    vec3 halfVector = normalize(view + lightPosition);
    
    float normalDotLight = clamp(dot(normal, lightPosition), 0.0, 1.0);
    float lightDotHalf = clamp(dot(lightPosition, halfVector), 0.0, 1.0);
    float normalDotHalf = clamp(dot(normal, halfVector), 0.0, 1.0);
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
    vec3 metallicRoughnessBSDF = diffuseBRDF + specularBRDF;
    
    float attenuation = SquareFalloffAttenuation(positionToLight, light.inverseFalloffRadius);
    attenuation *= mix(1.0, SpotAngleAttenuation(light, positionToLight), light.isSpot);
    
    vec3 luminance = light.color * normalDotLight * light.intensity * attenuation;
    o_color += vec4(metallicRoughnessBSDF * luminance * ambientOcclusion, 1.0);
  }
}
