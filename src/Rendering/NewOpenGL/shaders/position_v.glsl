#version 430 core
layout(location=0) in vec3 vertexPosition;

uniform mat4 MVPMatrix;

out vec3 texCoord;

void main() {
  gl_Position = MVPMatrix * vec4(vertexPosition, 1.0f);
  texCoord = vertexPosition;
}
