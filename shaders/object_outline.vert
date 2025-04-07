#version 460

layout (location=0) in vec3 vtx_position;
layout (location=1) in vec3 vtx_normal;
layout (location=2) in vec2 vtx_texcoord;

uniform mat4 m_persp;
uniform mat4 m_view;
uniform mat4 m_model;
uniform float outline_thickness = 0.01;

out vec3 normal;
out vec2 texcoord;

void main() {
    // Transform the normal to view space (only need rotation, not translation)
    mat3 normal_matrix = mat3(transpose(inverse(m_view * m_model)));
    vec3 view_normal = normalize(normal_matrix * vtx_normal);
    
    // Calculate position with normal extrusion in view space
    vec4 model_pos = m_model * vec4(vtx_position, 1.0);
    vec3 extrusion_dir = normalize(vtx_normal);
    vec4 extruded_pos = model_pos + vec4(extrusion_dir * outline_thickness, 0.0);
    
    // Final position in clip space
    gl_Position = m_persp * m_view * extruded_pos;
    
    normal = vtx_normal;
    texcoord = vtx_texcoord;
}
