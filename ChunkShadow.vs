#version 450

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 shadowView;
uniform mat4 shadowProjection;
out vec4 v_position;

void main() {
	gl_Position = shadowProjection*shadowView*model*vec4(pos,1);
	v_position  = gl_Position;
}
