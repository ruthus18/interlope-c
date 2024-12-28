#version 460

layout (location=0) in vec3 position;

uniform mat4 gm_modelview;
uniform mat4 gm_persp;

out vec4 var_color;

void main() {
    gl_Position = gm_persp * gm_modelview * vec4(position, 1.0);
    var_color = vec4(position, 1.0) * 0.6 + vec4(0.5, 0.5, 0.5, 0.5);
}
