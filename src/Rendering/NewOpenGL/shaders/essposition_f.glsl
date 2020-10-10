#version 430 core
in vec3 worldPos;
out vec4 fragColor;
void main()
{
    fragColor=vec4(worldPos,1.0);
}
