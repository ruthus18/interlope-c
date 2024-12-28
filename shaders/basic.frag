#version 460

out vec4 color;

uniform mat4 gm_modelview;
uniform mat4 gm_persp;

in vec4 var_color;

void main() {
    color = var_color;
}
