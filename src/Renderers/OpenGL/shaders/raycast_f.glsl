#version 430 core
highp in vec3 texCoord;
highp in vec4 rayCastDir;
highp in vec4 worldPos;
highp out vec4 fragColor;

layout(location=0) uniform sampler1D TF1;
layout(location=1) uniform sampler2D preIntTF;
layout(location=2) uniform sampler3D Block1;
uniform sampler1D TF2;
uniform sampler3D Block2;

layout(binding=1, rgba32f) uniform image2D entryPosTexture;
layout(binding=2, rgba32f) uniform image2D exitPosTexture;

uniform bool is_single_volume;
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
uniform float isoValue1;
uniform vec3 isoColor1;
uniform float isoValue2;
uniform vec3 isoColor2;
uniform vec3 bgColor;
uniform vec3 boundRatio;
uniform int width;
uniform int height;
highp vec3 rayDirection;

vec3 lightDir = cameraFront;
uniform float step;
uniform float voxel;

vec3 phongShading(vec3 samplePos, vec3 diffuseColor,int block_id) {
    vec3 N;

    if(block_id==1){
        if (!gradPreCal) {
            N.x = (texture(Block1, samplePos + vec3(voxel, 0, 0)).r - texture(Block1, samplePos + vec3(-voxel, 0, 0)).r);
            N.y = (texture(Block1, samplePos + vec3(0, voxel, 0)).r - texture(Block1, samplePos + vec3(0, -voxel, 0)).r);
            N.z = (texture(Block1, samplePos + vec3(0, 0, voxel)).r - texture(Block1, samplePos + vec3(0, 0, -voxel)).r);
        } else {
            N = texture(Block1, samplePos).xyz;
            N = N * 2.0 - 1.0;
        }
    }
    else if(block_id==2){
        if (!gradPreCal) {
            N.x = (texture(Block2, samplePos + vec3(voxel, 0, 0)).r - texture(Block2, samplePos + vec3(-voxel, 0, 0)).r);
            N.y = (texture(Block2, samplePos + vec3(0, voxel, 0)).r - texture(Block2, samplePos + vec3(0, -voxel, 0)).r);
            N.z = (texture(Block2, samplePos + vec3(0, 0, voxel)).r - texture(Block2, samplePos + vec3(0, 0, -voxel)).r);
        } else {
            N = texture(Block2, samplePos).xyz;
            N = N * 2.0 - 1.0;
        }
    }

    N = (model * vec4(N, 0.0f)).xyz;
    N = -normalize(N);
    //lightDir=(InverseModel*vec4(lightDir,0.0)).xyz;
    vec3 L = -normalize(lightDir);
    vec3 R = L;//rayDirection;

    vec3 ambient = ka*diffuseColor.rgb;
    vec3 specular = ks*pow(max(dot(N, (L+R)/2.0), 0.0), shininess)*vec3(1.0, 1.0, 1.0);
    vec3 diffuse = kd*max(dot(N, L), 0.0)*diffuseColor.rgb;
    vec3 radiance = pow(ambient+specular+diffuse,vec3(1.f/2.2f));
    return radiance;
}

float rand(vec3 pos) {
    return fract(sin(dot(pos, vec3(12.9898, 78.233, 24.1569)))*0.938)/5000.0;
}

vec4 render_mipmap(vec3 startPos, vec3 rayDirection, int steps, float step) {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 scalar = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < steps; i++) {
        vec4 tmp = texture(Block1, startPos);
        if (tmp.r > scalar.r) {
            scalar = vec4(tmp.r, tmp.r, tmp.r, tmp.r);
        }
        startPos += rayDirection * step;
    }
    color = scalar;
    return color;
}

vec4 render_isosurface(vec3 startPos, vec3 rayDirection, int steps, float step) {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    if(is_single_volume){
        float lastScalar;
        lastScalar=texture(Block1, startPos).r;
        for (int i = 0; i < steps; i++) {
            float tmp = texture(Block1, startPos).r;
            if (lastScalar <= isoValue1 && isoValue1 <= tmp) {
                color = vec4(isoColor1, 1.0);
                color.rgb = phongShading(startPos, color.rgb,1);
                return color;
            }
            lastScalar = tmp;

            startPos += rayDirection*step;
        }
    }
    else{
        float lastScalar1;
        float lastScalar2;
        lastScalar1=texture(Block1, startPos).r;
        lastScalar2=texture(Block2, startPos).r;
        for (int i = 0; i < steps; i++) {
            float tmp1 = texture(Block1, startPos).r;
            float tmp2 = texture(Block2, startPos).r;
            if (lastScalar1 <= isoValue1 && isoValue1 <= tmp1.r && lastScalar2 <= isoValue2 && isoValue2 <= tmp2.r) {

                color.rgb = phongShading(startPos, isoColor1,1)*0.5f+phongShading(startPos, isoColor2,2)*0.5f;
                color.a=1.f;
                return color;
            }
            else if(lastScalar1 <= isoValue1 && isoValue1 <= tmp1.r){
                color.rgb= phongShading(startPos,isoColor1,1);
                color.a=1.f;
                return color;
            }
            else if(lastScalar2 <= isoValue2 && isoValue2 <= tmp2.r){
                color.rgb= phongShading(startPos,isoColor2,2);
                color.a=1.f;
                return color;
            }
            lastScalar1 = tmp1;
            lastScalar2 = tmp2;
            startPos += rayDirection*step;
        }
    }
    return color;
}

vec4 render_volume(vec3 startPos, vec3 rayDirection, int steps, float step) {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    if(is_single_volume){
        for (int i = 0; i < steps; ++i) {
            if (startPos.x < 0.0 || startPos.y < 0.0 || startPos.z < 0.0) break;
            if (startPos.x > 1.0 || startPos.y > 1.0 || startPos.z > 1.0) break;
            vec3 samplePos = startPos + rayDirection * step * (float(i) + 0.5);
            vec4 scalar = texture(Block1, samplePos);
            vec4 sampleColor;
            if (scalar.r > 0.0f) {
                sampleColor = texture(TF1, scalar.r);
                if(sampleColor.a>0.f){
                    sampleColor.rgb = phongShading(samplePos, sampleColor.rgb,1);
                    color = color + sampleColor * vec4(sampleColor.aaa, 1.0) * (1.0 - color.a);
                    if (color.a > 0.99) {
                        break;
                    }
                }
            }
        }
    }
    else{
        for (int i = 0; i < steps; ++i) {
            if (startPos.x < 0.0 || startPos.y < 0.0 || startPos.z < 0.0) break;
            if (startPos.x > 1.0 || startPos.y > 1.0 || startPos.z > 1.0) break;

            vec3 samplePos = startPos + rayDirection * step * (float(i) + 0.5);
            //samplePos.z*=2.0;//!!!
            // samplePos *= boundRatio;
            vec4 scalar1 = texture(Block1, samplePos);
            vec4 scalar2 = texture(Block2, samplePos);
            vec4 sampleColor1;
            vec4 sampleColor2;
            if (scalar1.r > 0.0f || scalar2.r>0.f) {
                sampleColor1 = texture(TF1, scalar1.r);
                sampleColor2 = texture(TF2, scalar2.r);
                if(sampleColor1.a >0.f || sampleColor2.a>0.f){
                    sampleColor1.rgb = phongShading(samplePos, sampleColor1.rgb,1);
                    sampleColor2.rgb = phongShading(samplePos, sampleColor2.rgb,2);
                    vec4 sampleColor=sampleColor1*sampleColor1.a+sampleColor2*sampleColor2.a;
                    color = color + sampleColor * vec4(sampleColor.aaa, 1.0) * (1.0 - color.a);
                    if (color.a > 0.99) {
                        break;
                    }
                }
            }
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
