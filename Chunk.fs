
#version 450

out vec4 FragColor;
in vec3 Norm;

void main() {
	FragColor = vec4(Norm*0.5+0.5,1.0);

}