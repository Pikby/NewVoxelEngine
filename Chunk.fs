
#version 450

out vec4 FragColor;
in vec3 Norm;
in vec4 Color;
in vec3 WorldPos;

uniform float time;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;

uniform float specularExponent;

uniform vec3 globalLightDir;
uniform vec3 cameraPos;
uniform int translucent;

uniform sampler2D waterTexture;

vec3 calculateGlobalColor(){
	vec3 diffuse = max(dot(Norm, globalLightDir),0.0)*diffuseColor;
	vec3 viewDir = normalize(cameraPos-WorldPos);
	vec3 reflectDir = reflect(-globalLightDir,Norm);
	float spec = pow(max(dot(viewDir,reflectDir),0),specularExponent);
	vec3 specular = 0.5*spec*specularColor;

	return ambientColor+diffuse+specular;
}

void main() {



	vec3 globalColor = calculateGlobalColor();
	FragColor = vec4((globalColor)*Color.rgb,Color.a);
	if(translucent == 1){
	vec2 textureOrigin = cameraPos.xz*4-vec2(512);
	vec2 textureCoord = ( WorldPos.xz - textureOrigin);
		FragColor = FragColor*(max((texture(waterTexture,((WorldPos.xz +vec2(time*10))/1024.0f)).r+0.5),1));
	}
}