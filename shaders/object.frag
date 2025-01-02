#version 460

out vec4 fragColor;

in vec3 normal;
in vec2 texcoord;


void main() {
    fragColor = vec4(normal, 1.0);
}
