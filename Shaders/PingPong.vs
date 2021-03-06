#version 450
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;


uniform mat4 globalScreenProjection;
out vec2 TexCoords;


void main(){
	gl_Position = vec4(pos,1);
	TexCoords = tex;
}