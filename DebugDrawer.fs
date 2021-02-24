#version 450
out vec4 FragColor;
in vec2 Tex;

uniform float time;
uniform vec2 windDirection;
uniform float cloudSpeed;
uniform sampler2D cloudTexture;
void main()
{


    vec2 offset = windDirection*time*cloudSpeed;
    float alpha = texture(cloudTexture,((Tex + offset)/1024.0f)).r;
    alpha = smoothstep(0.5,1.0,alpha);

    FragColor = vec4(vec3(1.0),alpha);
    FragColor = vec4(1,0,0,1);
}