#version 430 core
layout(location=0) in vec3 vertexPos;
out vec3 worldPos;
uniform mat4 MVPMatrix;

void main()
{
    gl_Position=MVPMatrix*vec4(vertexPos,1.0);
    worldPos=vertexPos;
}
