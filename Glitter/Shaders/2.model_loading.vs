#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float gl_ClipDistance[1];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clip_coord;

void main()
{
     gl_ClipDistance[0] = dot(model*vec4(aPos, 1.0), clip_coord);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}