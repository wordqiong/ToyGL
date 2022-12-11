#version 330 core

#define NR_POINT_LIGHTS 4
out vec4 FragColor;

struct Material {

    vec3 specular;    
    float shininess;
}; 

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  





uniform vec3 viewPos;
uniform Material material;
uniform sampler2D texture_diffuse1;
uniform DirLight dirLight;



in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);


void main()
{    
     
     // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 第一阶段：定向光照
    vec3 result = CalcDirLight(dirLight, norm, viewDir);


     // 第三阶段：聚光
    
    // vec3 lightDir = normalize(light.position - FragPos);
    FragColor = vec4(result, 1.0);
   
}



// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(material.specular);
    return (ambient + diffuse + specular);
}