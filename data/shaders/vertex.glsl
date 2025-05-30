#version 460

layout (location = 0) in uvec3 packedPosition;
layout (location = 1) in ivec3 packedNormal;
layout (location = 2) in ivec4 packedTangent;
layout (location = 3) in uvec2 packedUV;

layout (location = 0) uniform mat4 mvp;
layout (location = 4) uniform mat4 modelMatrix;
layout (location = 8) uniform mat3 normalMatrix;
layout (location = 11) uniform vec2 uvScale;
layout (location = 12) uniform vec2 uvOffset;
layout (location = 13) uniform vec3 cameraPosition;
layout (location = 14) uniform vec4 baseColorFactor;
layout (location = 15) uniform float metallicFactor;
layout (location = 16) uniform float roughnessFactor;

layout (location = 0) out vec3 o_position;
layout (location = 1) out vec3 o_cameraPosition;
layout (location = 2) out vec2 o_uv;
layout (location = 3) out vec4 o_baseColorFactor;
layout (location = 4) out float o_metallicFactor;
layout (location = 5) out float o_roughnessFactor;
layout (location = 6) out mat3 o_tbn;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  vec4 unpackedPosition = vec4(packedPosition.xyz / 65535.0, 1.0);
  vec4 worldPosition = modelMatrix * unpackedPosition;
  vec4 projectedPosition = mvp * unpackedPosition;
  
  vec3 unpackedNormal = max(packedNormal / 127.0, -1.0);
  vec4 unpackedTangent = max(packedTangent / 127.0, -1.0);
  
  mat3 uvTransform = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, uvOffset.x, uvOffset.y, 1.0) *
                     mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0) *
                     mat3(uvScale.x, 0.0, 0.0, 0.0, uvScale.y, 0.0, 0.0, 0.0, 1.0);
  vec2 unpackedUV = (uvTransform * vec3(packedUV.xy / 65535.0, 1.0)).xy;
  
  vec3 view =  normalize(cameraPosition - worldPosition.xyz);

  vec3 normal = normalize(normalMatrix * unpackedNormal);
  vec3 tangent = normalize(normalMatrix * unpackedTangent.xyz);
  tangent = normalize(tangent - dot(tangent, normal) * normal); // Applying Gram-Schmidt process to re-orthogonalize the TBN vectors
  float handedness = unpackedTangent.w;
  vec3 bitangent = normalize(normalMatrix * vec3(cross(normal, tangent) * handedness));
  mat3 tbn = mat3(tangent, bitangent, normal);

  gl_Position = projectedPosition;
  o_position = worldPosition.xyz;
  o_cameraPosition = cameraPosition;
  o_uv = unpackedUV;
  o_baseColorFactor = baseColorFactor;
  o_metallicFactor = metallicFactor;
  o_roughnessFactor = roughnessFactor;  
  o_tbn = tbn;
}
