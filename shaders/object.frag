#version 460


// Buffers
in vec3 normals;
in vec2 texcoords;

// Textures
// layout (binding=0) uniform sampler2D texture_diff;
// layout (binding=1) uniform sampler2D texture_normal;

out vec4 fragColor;


void main() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // fragColor = texture(texture_diff, texcoords);
    // fragColor = texture(texture_diff, texcoords) * vec4(light.color + material.diffuse_color, 1.0);
    // fragColor = texture(texture_diff, texcoords) * vec4(light.color + material.diffuse_color + 1.0, 1.0);
}
