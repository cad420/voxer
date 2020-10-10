#version 430 core
layout(location=0) in vec3 vertexPos;
uniform mat4 MVPMatrix;
uniform mat4 model;
uniform mat4 view;
out vec4 rayCastDir;
void main()
{
    gl_Position = MVPMatrix*vec4(vertexPos,1.0);
    rayCastDir= view*model*vec4(vertexPos,1.0);//assert no model transform
    rayCastDir.w=0.0;
}

