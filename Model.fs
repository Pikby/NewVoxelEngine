#version 330 core

out vec4 FragColor;
in vec3 Norm;
in vec2 Tex;

uniform sampler2D objTexture;
void main()
{
    vec3 color = Norm*0.5 +0.5;
    FragColor = vec4(color,1);
}