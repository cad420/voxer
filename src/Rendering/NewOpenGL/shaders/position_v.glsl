#version 430 core
layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec3 texturePosition;
uniform mat4 MVPMatrix;
out vec3 texCoord;
void main()
{
    gl_Position=MVPMatrix*vec4(vertexPosition,1.0);
    texCoord=vertexPosition;
}
