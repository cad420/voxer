#version 430 core
in vec3 texCoord;

out vec4 fragColor;

void main() {
  fragColor = vec4(texCoord, 1.0);
}
