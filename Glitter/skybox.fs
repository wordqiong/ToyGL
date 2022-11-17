#version 330 core
out vec4 FragColor;
in vec3 TexCoord;
uniform samplerCube sky;
void main()
{
    FragColor = texture(sky, TexCoord);
}