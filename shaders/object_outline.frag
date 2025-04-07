#version 460

out vec4 fragColor;

in vec3 normal;
in vec2 texcoord;

uniform vec3 outline_color1 = vec3(0.9, 0.6, 0.0);
uniform vec3 outline_color2 = vec3(0.25, 0.7, 0.0);
uniform float outline_intensity = 1.0;


void main() {
    float gradient_factor = (normal.y + 1.0) * 0.5;
    vec3 outline_color = mix(outline_color1, outline_color2, gradient_factor);
    
    fragColor = vec4(outline_color, outline_intensity);
}
