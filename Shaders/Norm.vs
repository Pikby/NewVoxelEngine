#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;


out VS_OUT {
    vec3 norm;
} vs_out;


void main() {
	gl_Position = vec4(pos,1);
	vs_out.norm = norm;
}
