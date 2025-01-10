#version 460

out vec4 fragColor;

in vec3 normal;
in vec2 texcoord;

layout (binding=0) uniform sampler2D texture_diff;


void main() {
    fragColor = texture(texture_diff, texcoord);
}
