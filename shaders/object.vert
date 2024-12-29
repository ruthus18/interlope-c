#version 460

layout (location=0) in vec3 vertex_buffer;
layout (location=1) in vec3 normal_buffer;
layout (location=2) in vec2 texcoord_buffer;

uniform mat4 m_persp;
uniform mat4 m_view;
uniform mat4 m_model;

out vec3 normals;
out vec2 texcoords;


void main() {
    gl_Position = m_persp * m_view * m_model * vec4(vertex_buffer, 1.0);

    normals = normal_buffer;
    texcoords = texcoord_buffer;
}
