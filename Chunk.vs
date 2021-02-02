#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in int color;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 Norm;
out vec4 Color;
out vec3 WorldPos;


float compressedColorToFloat(int compressed, int byteoffset){
return ((compressed >> byteoffset) & 0xf)/float(0xf);
}



void main() {
	Color = vec4(compressedColorToFloat(color,0),compressedColorToFloat(color,4),compressedColorToFloat(color,8),compressedColorToFloat(color,12));


	gl_Position = projection*view*model*vec4(pos,1);
	WorldPos = (model*vec4(pos,1)).xyz;
	Norm = norm;
}
