#version 460

layout (location = 0) in vec4 vertex;  // <vec2 pos, vec2 tex>
out vec2 tex_coords;

uniform mat4 projection;


void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    tex_coords = vertex.zw;
}
