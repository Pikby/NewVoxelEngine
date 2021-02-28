#version 330 core

out vec4 FragColor;
in vec3 Norm;
in vec2 Tex;

uniform sampler2D objTexture;
void main()
{

    FragColor = vec4(0.380, 0.180, 0,1);
}