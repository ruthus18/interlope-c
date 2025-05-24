#version 460

layout (location=0) in vec3 vtx_position;

uniform mat4 m_persp;
uniform mat4 m_view;
uniform mat4 m_model;
uniform vec3 v_color;

out vec3 frag_color;


void main() {
    gl_Position = m_persp * m_view * m_model * vec4(vtx_position, 1.0);
    frag_color = v_color;
}
