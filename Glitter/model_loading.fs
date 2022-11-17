#version 330 core

#define NR_POINT_LIGHTS 4
out vec4 FragColor;

struct Material {

    vec3 specular;    
    float shininess;
}; 

uniform vec3 viewPos;
uniform Material material;
uniform sampler2D texture_diffuse1;
uniform samplerCube sky;

uniform vec3 cameraPos;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;


void main()
{    
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(FragPos - cameraPos);
    vec3 R = refract(I, normalize(Normal),ratio);
    FragColor = vec4(texture(sky, R).rgb, 1.0);
   
}

