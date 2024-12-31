#version 460


uniform vec3 position;
uniform mat4 m_persp;
uniform mat4 m_view;
uniform mat4 m_model;

out mat4 test;


void main() {
    gl_Position = m_persp * m_view * m_model * vec4(position, 1.0);
}
