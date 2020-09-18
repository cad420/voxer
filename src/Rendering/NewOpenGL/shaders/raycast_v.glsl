#version 430 core
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
layout(location = 0) in vec3 vertexPos;
highp out vec3 texCoord;//纹理坐标，从vshader到fshader，会为每一个fragment通过插值生成纹理坐标，一般来说，裁减后的世界中 片段比顶点更多
highp out vec4 rayCastDir;//顶点的相机观察系坐标，即顶点的位置就是光线投射的方向（相机在原点），到fragment也会生成每个片段在观察系下的坐标
highp out vec4 worldPos;
void main() {
    gl_Position = projection*view*model*vec4(vertexPos,1.0);
    texCoord = vertexPos;
    rayCastDir= view*model*vec4(vertexPos,1.0);//assert no model transform
    rayCastDir.w=0.0;
    worldPos=model*vec4(vertexPos,1.0);
}
