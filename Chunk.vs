#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
uniform mat4 projection;
uniform mat4 view;
out vec3 Norm;

void main() {
	gl_Position = projection*view*vec4(pos,1);
	Norm = norm;
}
