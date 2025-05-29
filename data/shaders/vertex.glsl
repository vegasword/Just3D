#version 460

layout (location = 0) in uvec3 packedPosition;
layout (location = 1) in ivec3 packedNormal;
layout (location = 2) in ivec4 packedTangent;
layout (location = 3) in uvec2 packedUV;

layout (location = 0) uniform mat4 mvp;
layout (location = 1) uniform mat3 normalMatrix;
layout (location = 2) uniform vec2 uvScale;
layout (location = 3) uniform vec2 uvOffset;
layout (location = 4) uniform vec3 cameraPosition;
layout (location = 5) uniform vec4 baseColorFactor;
layout (location = 6) uniform float metallicFactor;
layout (location = 7) uniform float roughnessFactor;

out vec3 o_position;
out vec2 o_uv;
out vec4 o_baseColorFactor;
out float o_metallicFactor;
out float o_roughnessFactor;
out vec3 o_view;
out vec3 o_light;
out vec3 o_half;
out mat3 o_tbn;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  vec4 unpackedPosition = vec4(packedPosition.xyz / 65535.0, 1.0);
  vec4 worldPosition = mvp * unpackedPosition;
  
  vec3 unpackedNormal = max(packedNormal / 127.0, -1.0);
  vec4 unpackedTangent = max(packedTangent / 127.0, -1.0);
  
  mat3 uvTransform = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, uvOffset.x, uvOffset.y, 1.0) *
                     mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0) *
                     mat3(uvScale.x, 0.0, 0.0, 0.0, uvScale.y, 0.0, 0.0, 0.0, 1.0);
  vec2 unpackedUV = (uvTransform * vec3(packedUV.xy / 65535.0, 1.0)).xy;

  vec3 view =  normalize(cameraPosition - worldPosition.xyz);
  vec3 light = vec3(-1.0, 1.0, -1.0); // Directional for now
  
  vec3 halfVector = normalize(view + light);

  vec3 normal = normalize(normalMatrix * unpackedNormal);
  vec3 tangent = normalize(normalMatrix * unpackedTangent.xyz);
  tangent = normalize(tangent - dot(tangent, normal) * normal); // Applying Gram-Schmidt process to re-orthogonalize the TBN vectors
  float handedness = unpackedTangent.w;
  vec3 bitangent = normalize(normalMatrix * vec3(cross(normal, tangent) * handedness));
  mat3 tbn = mat3(tangent, bitangent, normal);

  gl_Position = worldPosition;
  o_position = worldPosition.xyz;
  o_uv = unpackedUV;
  o_baseColorFactor = baseColorFactor;
  o_metallicFactor = metallicFactor;
  o_roughnessFactor = roughnessFactor;  
  o_view = view;
  o_light = light;
  o_half = halfVector;
  o_tbn = tbn;
}
