#version 460

in vec2 uv;

uniform sampler2D modelTexture;

out vec4 o_color;

void main()
{
  o_color = texture(modelTexture, uv); 
}
