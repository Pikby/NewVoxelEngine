#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in int color;
uniform mat4 globalProjection;
uniform mat4 globalView;
uniform mat4 model;

uniform mat4 shadowView;
uniform mat4 shadowProjection;
out vec3 Norm;
out vec4 Color;
out vec3 WorldPos;
out vec4 DirectionalShadowPos;

float compressedColorToFloat(int compressed, int byteoffset){
return ((compressed >> byteoffset) & 0xff)/float(0xff);
} 



void main() {
	Color = vec4(compressedColorToFloat(color,0),compressedColorToFloat(color,8),compressedColorToFloat(color,16),compressedColorToFloat(color,24));


	gl_Position = globalProjection*globalView*model*vec4(pos,1);
	WorldPos = (model*vec4(pos,1)).xyz;
	Norm = norm;
	DirectionalShadowPos = shadowProjection*shadowView*vec4(WorldPos,1.0);
}
