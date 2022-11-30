#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
void main()
{             
    
    BrightColor= vec4( 0.3,0.3,0.3,0.1);
}
