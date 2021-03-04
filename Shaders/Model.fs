#version 330 core

out vec4 FragColor;
in vec3 Norm;
in vec2 Tex;

uniform vec3 objColor;
uniform sampler2D objTexture;
void main()
{

    FragColor = vec4(objColor,1);
}