#version 460

layout (location=0) in vec3 vtx_position;
layout (location=1) in vec3 vtx_normal;
layout (location=2) in vec2 vtx_texcoord;

uniform mat4 m_persp;
uniform mat4 m_view;
uniform mat4 m_model;

out vec3 normal;
out vec2 texcoord;


void main() {
    gl_Position = m_persp * m_view * m_model * vec4(vtx_position, 1.0);

    normal = vtx_normal;
    texcoord = texcoord;
}
