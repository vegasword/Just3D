#version 460

layout (location = 0) in uvec3 position;
layout (location = 1) in ivec3 normal;
layout (location = 2) in uvec2 uv;

layout (location = 0) uniform mat4 mvp;
layout (location = 1) uniform vec2 uvScale;
layout (location = 2) uniform vec2 uvOffset;

out vec2 o_uv;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  vec4 unpackedPosition = vec4(position.xyz / 65535.0, 1.0);
  
  mat3 uvTransform = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, uvOffset.x, uvOffset.y, 1.0) *
                     mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0) *
                     mat3(uvScale.x, 0.0, 0.0, 0.0, uvScale.y, 0.0, 0.0, 0.0, 1.0);
                 
  gl_Position = mvp * unpackedPosition;
  // o_normal = vec4(max(normal / 127.0, -1.0), 1.0);
  o_uv = (uvTransform * vec3(uv.xy / 65535.0, 1.0)).xy;
}
