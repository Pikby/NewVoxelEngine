#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;


in VS_OUT {
    vec3 norm;
} gs_in[];

const float MAGNITUDE = 0.4;
  
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void GenerateLine(int index) {
    gl_Position = projection * view * model * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * view * model* (gl_in[index].gl_Position + vec4(gs_in[index].norm.xyz, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
} 