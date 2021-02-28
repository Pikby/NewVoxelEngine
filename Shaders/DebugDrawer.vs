#version 450
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;


uniform mat4 globalProjection;
uniform mat4 globalView;
uniform mat4 model;
out vec2 Tex;


void main(){
	gl_Position = globalProjection*globalView*vec4(pos,1);
	Tex = (model*vec4(pos,1)).xz;
	
}