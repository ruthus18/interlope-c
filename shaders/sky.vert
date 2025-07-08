#version 460

layout (location = 0) in vec3 vtx_position;

out vec3 texcoord;

uniform mat4 m_persp;
uniform mat4 m_view;


void main() {
    gl_Position = m_persp * m_view * vec4(vtx_position, 1.0);

    texcoord = vtx_position;
}  
