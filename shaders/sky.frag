#version 460

out vec4 FragColor;
in vec3 texcoord;

uniform samplerCube skybox;


void main()
{    
    FragColor = texture(skybox, texcoord);
}
