#version 430 core
highp in vec3 texCoord;
highp in vec4 rayCastDir;
highp in vec4 worldPos;
highp out vec4 fragColor;

layout(location=0) uniform sampler1D TF;
layout(location=1) uniform sampler2D preIntTF;
layout(location=2) uniform sampler3D Block;

layout(binding=1, rgba32f) uniform image2D entryPosTexture;
layout(binding=2, rgba32f) uniform image2D exitPosTexture;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 lightPos;
uniform bool inner;
uniform mat4 model;
highp uniform mat4 InverseView;
highp uniform mat4 InverseModel;
highp uniform vec3 cameraPos;
uniform bool isPerspective;
uniform bool usePreIntTF;
highp uniform vec3 cameraFront;
uniform bool gradPreCal;
uniform bool isMipMap;
// 0 for mip , 1 for isovalue , 2 for normal tf draw
uniform int drawMode;
uniform float isoValue;
uniform vec3 isoColor;
uniform vec3 bgColor;
uniform vec3 boundRatio;
uniform int width;
uniform int height;
highp vec3 rayDirection;

vec3 lightDir = cameraFront;
const float step= 1.0 / 256 * 0.3;

vec3 phongShading(vec3 samplePos, vec3 diffuseColor) {
  vec3 N;
  if (!gradPreCal) {
    N.x = (texture(Block, samplePos + vec3(step, 0, 0)).r - texture(Block, samplePos + vec3(-step, 0, 0)).r);
    N.y = (texture(Block, samplePos + vec3(0, step, 0)).r - texture(Block, samplePos + vec3(0, -step, 0)).r);
    N.z = (texture(Block, samplePos + vec3(0, 0, step)).r - texture(Block, samplePos + vec3(0, 0, -step)).r);
  } else {
    N = texture(Block, samplePos).xyz;
    N = N * 2.0 - 1.0;
  }

  N = (model * vec4(N, 0.0f)).xyz;
  N = -normalize(N);
  //lightDir=(InverseModel*vec4(lightDir,0.0)).xyz;
  vec3 L = -normalize(lightDir);
  vec3 R = L;//rayDirection;

  vec3 ambient = ka*diffuseColor.rgb;
  vec3 specular = ks*pow(max(dot(N, (L+R)/2.0), 0.0), shininess)*vec3(1.0, 1.0, 1.0);
  vec3 diffuse = kd*max(dot(N, L), 0.0)*diffuseColor.rgb;
  return ambient + specular + diffuse;
}

float rand(vec3 pos) {
  return fract(sin(dot(pos, vec3(12.9898, 78.233, 24.1569)))*0.938)/5000.0;
}

vec4 render_mipmap(vec3 startPos, vec3 rayDirection, int steps, float step) {
  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 scalar = vec4(0.0, 0.0, 0.0, 0.0);
  for (int i = 0; i < steps; i++) {
    vec4 tmp = texture(Block, startPos);
    if (gradPreCal) {
      if (tmp.a > scalar.a) {
        scalar = vec4(tmp.a, tmp.a, tmp.a, tmp.a);
      }
    } else {
      if (tmp.r > scalar.r) {
        scalar = vec4(tmp.r, tmp.r, tmp.r, tmp.r);
      }
    }
    startPos += rayDirection * step;
  }
  color = scalar;
  return color;
}

vec4 render_isosurface(vec3 startPos, vec3 rayDirection, int steps, float step) {
  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  float lastScalar;
  if (gradPreCal) {
    lastScalar=texture(Block, startPos).a;
  } else {
    lastScalar=texture(Block, startPos).r;
  }

  for (int i = 0; i < steps; i++) {
    vec4 tmp = texture(Block, startPos);
    if (gradPreCal) {
      if (lastScalar <= isoValue && isoValue <= tmp.a) {
        color = vec4(isoColor, 1.0);
        color.rgb = phongShading(startPos, color.rgb);
        return color;
      }
      lastScalar = tmp.a;
    } else {
      if (lastScalar <= isoValue && isoValue <= tmp.r) {
        color = vec4(isoColor, 1.0);
        color.rgb = phongShading(startPos, color.rgb);
        return color;
      }
      lastScalar = tmp.r;
    }
    startPos += rayDirection*step;
  }
  return color;
}

vec4 render_volume(vec3 startPos, vec3 rayDirection, int steps, float step) {
  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 oldScalar = texture(Block, startPos);
  for (int i = 0; i < steps; ++i) {
    if (startPos.x < 0.0 || startPos.y < 0.0 || startPos.z < 0.0) break;
    if (startPos.x > 1.0 || startPos.y > 1.0 || startPos.z > 1.0) break;

    vec3 samplePos = startPos + rayDirection * step * (float(i) + 0.5);
    //samplePos.z*=2.0;//!!!
    // samplePos *= boundRatio;
    vec4 scalar = texture(Block, samplePos);
    vec4 sampleColor;
    if (scalar.r > 0.0f) {
      if (!gradPreCal){
        if (usePreIntTF){
          sampleColor = texture(preIntTF, vec2(oldScalar.r, scalar.r));
        } else {
          sampleColor = texture(TF, scalar.r);
        }
      } else {
        if (usePreIntTF) {
          sampleColor = texture(preIntTF, vec2(oldScalar.a, scalar.a));
        } else {
          sampleColor = texture(TF, scalar.a);
        }
      }
      sampleColor.rgb = phongShading(samplePos, sampleColor.rgb);
      color = color + sampleColor * vec4(sampleColor.aaa, 1.0) * (1.0 - color.a);
      if (color.a > 0.99) {
        break;
      }
      oldScalar = scalar;
    }
  }
  return color;
}

vec4 calColor() {
  ivec2 screenCoord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
  vec3 startPos = imageLoad(entryPosTexture, screenCoord).xyz;
  vec3 exitPos = imageLoad(exitPosTexture, screenCoord).xyz;

  rayDirection = normalize(exitPos - startPos);
  float distance = dot((exitPos - startPos), rayDirection);

  //  if (inner) {
  //    startPos = cameraPos;
  //  } else {
  //    startPos = (InverseModel * worldPos).xyz;
  //  }
  //
  //  if (isPerspective) {
  //    rayDirection = normalize((InverseModel * InverseView * rayCastDir).xyz);
  //  } else {
  //    rayDirection = normalize((InverseModel * vec4(cameraFront, 0.0)).xyz);
  //  }

  int steps = int(distance / step);

  if (drawMode == 0) {
    return render_mipmap(startPos, rayDirection, steps, step);
  }

  if (drawMode == 1) {
    return render_isosurface(startPos, rayDirection, steps, step);
  }

  return render_volume(startPos, rayDirection, steps, step);
}

void main() {
  vec4 color = calColor();
  if (color.a == 0.0) {
    discard;
  }

  vec3 bg = bgColor;
  if (isMipMap) {
    bg = vec3(0.0, 0.0, 0.0);
  }
  color = color + vec4(bg, 0.0) * (1.0 - color.a);
  color.a = 1.0;
  fragColor = color;
}
