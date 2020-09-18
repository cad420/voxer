#version 430 core
layout(location=0) in vec2 vertexPos;
layout(location=1) in vec2 texCoord;
out vec2 screenTexCoord;
void main() {
    gl_Position=vec4(vertexPos,0.0,1.0);
    screenTexCoord=texCoord;
}
