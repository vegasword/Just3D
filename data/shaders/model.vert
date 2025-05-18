#version 460

layout (location = 0) in uvec3 position;
layout (location = 1) in ivec3 normal;
layout (location = 2) in ivec4 tangent;
layout (location = 3) in uvec2 uv;

layout (location = 0) uniform mat4 mvp;
layout (location = 1) uniform mat3 modelMatrix;
layout (location = 2) uniform vec2 uvScale;
layout (location = 3) uniform vec2 uvOffset;
layout (location = 4) uniform vec3 cameraPosition;
layout (location = 5) uniform vec4 baseColorFactor;
layout (location = 6) uniform float metallicFactor;
layout (location = 7) uniform float roughnessFactor;

out vec3 o_vertexPosition;
out vec2 o_vertexUV;
out vec4 o_baseColorFactor;
out float o_metallicFactor;
out float o_roughnessFactor;
out vec3 o_cameraPosition;
out mat3 o_tbn;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  vec4 unpackedPosition = vec4(position.xyz / 65535.0, 1.0);
  vec3 unpackedNormal = max(normal / 127.0, -1.0);
  vec4 unpackedTangent = max(tangent / 127.0, -1.0);
  
  mat3 uvTransform = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, uvOffset.x, uvOffset.y, 1.0) *
                     mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0) *
                     mat3(uvScale.x, 0.0, 0.0, 0.0, uvScale.y, 0.0, 0.0, 0.0, 1.0);
                   
  vec2 unpackedUV = (uvTransform * vec3(uv.xy / 65535.0, 1.0)).xy;
                   
  gl_Position = mvp * unpackedPosition;
  
  vec3 vertexPosition = gl_Position.xyz;
  float handedness = unpackedTangent.w;
  
  vec3 viewVector = normalize(cameraPosition - vertexPosition);
  vec3 lightVector = normalize(vec3(1.0,1.0,1.0) - vertexPosition);
  vec3 halfVector = normalize(lightVector + viewVector);

  vec3 normal = normalize(modelMatrix * unpackedNormal);
  vec3 tangent = normalize(modelMatrix * unpackedTangent.xyz);
  tangent = normalize(tangent - dot(tangent, normal) * normal); // Applying Gram-Schmidt process to re-orthogonalize the TBN vectors
  vec3 bitangent = normalize(modelMatrix * vec3(cross(normal, tangent) * handedness));
  mat3 tbn = mat3(tangent, bitangent, normal);

  o_vertexUV = unpackedUV;
  o_cameraPosition = cameraPosition;
  o_baseColorFactor = baseColorFactor;
  o_metallicFactor = metallicFactor;
  o_roughnessFactor = roughnessFactor;
  o_tbn = tbn;
}
