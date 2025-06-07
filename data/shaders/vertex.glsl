#version 460

layout (location = 0) in uvec3 packedVertexPosition;
layout (location = 1) in ivec3 packedVertexNormal;
layout (location = 2) in ivec4 packedVertexTangent;
layout (location = 3) in uvec2 packedVertexUV;

layout (std140, binding = 4) uniform Uniforms
{
  mat4 mvp;
  mat4 modelMatrix;
  vec4 normalMatrixFirstColumn;
  vec4 normalMatrixSecondColumn;
  vec4 normalMatrixThirdColumn;
  vec4 baseColor;
  vec4 cameraPosition;
  vec2 uvScale;
  vec2 uvOffset;
  float metallicFactor;
  float roughnessFactor;
} uniforms;

layout (location = 0) out vec3 o_worldVertexPosition;
layout (location = 1) out vec3 o_cameraPosition;
layout (location = 2) out vec2 o_uv;
layout (location = 3) out vec4 o_baseColor;
layout (location = 4) out float o_metallicFactor;
layout (location = 5) out float o_roughnessFactor;
layout (location = 6) out mat3 o_tbn;

out gl_PerVertex { vec4 gl_Position; };

void main()
{    
  vec4 localVertexPosition = vec4(packedVertexPosition.xyz / 65535.0, 1.0);  
  vec3 localNormal = clamp(packedVertexNormal / 127.0, -1.0, 1.0);
  vec4 localTangent = clamp(packedVertexTangent / 127.0, -1.0, 1.0);
  
  vec2 uv = packedVertexUV.xy / 65535.0;
  uv *= uniforms.uvScale;
  uv += uniforms.uvOffset;
  
  vec4 worldVertexPosition = uniforms.modelMatrix * localVertexPosition;
  vec4 projectedPosition = uniforms.mvp * localVertexPosition;
  vec3 view = normalize(uniforms.cameraPosition.xyz - worldVertexPosition.xyz);

  mat3 normalMatrix = mat3(
    uniforms.normalMatrixFirstColumn.xyz,
    uniforms.normalMatrixSecondColumn.xyz,
    uniforms.normalMatrixThirdColumn.xyz
  );

  vec3 worldNormal = normalize(normalMatrix * localNormal);
  vec3 worldTangent = normalize(normalMatrix * localTangent.xyz);
  worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal); // Applying Gram-Schmidt process to re-orthogonalize the TBN vectors
  float handedness = localTangent.w;
  vec3 bitangent = normalize(normalMatrix * cross(worldNormal, worldTangent)) * handedness;
  mat3 tbn = mat3(worldTangent, bitangent, worldNormal);

  gl_Position = projectedPosition;
  
  o_worldVertexPosition = worldVertexPosition.xyz;
  o_cameraPosition = uniforms.cameraPosition.xyz;
  o_uv = uv;
  o_baseColor = uniforms.baseColor;
  o_metallicFactor = uniforms.metallicFactor;
  o_roughnessFactor = uniforms.roughnessFactor;  
  o_tbn = tbn;
}
