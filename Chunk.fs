
#version 450

out vec4 FragColor;
in vec3 Norm;
in vec4 Color;
in vec3 WorldPos;

//uniform vec3 globalLightDir;
vec3 globalLightDir = vec3(0.1,0.5,0.1);
uniform vec3 cameraPos;
vec3 ambient = vec3(0.5,0.5,0.5);
void main() {

	vec3 diffuse = max(dot(Norm, globalLightDir),0.0)*vec3(1);
	vec3 viewDir = normalize(cameraPos-WorldPos);
	vec3 reflectDir = reflect(-globalLightDir,Norm);
	float spec = pow(max(dot(viewDir,reflectDir),0),32);
	vec3 specular = 0.5*spec*vec3(1);

	vec3 color = Color.rgb;
	float alpha = Color.a;
	FragColor = vec4((ambient+diffuse+specular)*color,alpha);
	//FragColor= vec4(Norm*0.5+0.5,1);
}