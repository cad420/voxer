#version 430 core

layout(location = 0, rgba32f) uniform volatile image2D entryPosTexture;
layout(location = 1, rgba32f) uniform volatile image2D exitPosTexture;
layout(location = 2, rgba32f) uniform volatile image2D resutlTexture;
layout(location = 3) uniform sampler1D texTransfunc;
layout(location = 4) uniform sampler3D volumeTexture;
layout(location = 5) uniform int render_volume;
layout(location = 6) uniform float isovalue;
layout(location = 7) uniform vec3 isosurface_color;

in vec2 screenCoord;
out vec4 fragColor;
float step = 0.01;

vec3 gradient(in sampler3D s, vec3 p, float dt) {
    vec2 e = vec2(dt, 0.0);

    return vec3(
    texture(s, p - e.xyy).r - texture(s, p + e.xyy).r,
    texture(s, p - e.yxy).r - texture(s, p + e.yxy).r,
    texture(s, p - e.yyx).r - texture(s, p + e.yyx).r
    );
}

void main()
{
    vec3 rayStart = vec3(imageLoad(entryPosTexture, ivec2(gl_FragCoord)).xyz);
    vec3 rayEnd = vec3(imageLoad(exitPosTexture, ivec2(gl_FragCoord)).xyz);

    vec3 start2end = vec3(rayEnd - rayStart);
    vec4 color = imageLoad(resutlTexture, ivec2(gl_FragCoord)).xyzw;
    vec4 bg = vec4(0.f, 0.f, 0.f, .00f);
    vec3 direction = normalize(start2end);
    float distance = dot(direction, start2end);
    int steps = int(distance / step);
    vec3 samplePoint = rayStart;

    float prev_value = texture(volumeTexture, samplePoint).r;
    for (int i = 0; i < steps; ++i) {
        samplePoint += direction * step;
        vec4 scalar = texture(volumeTexture, samplePoint);

        if (render_volume == 1) {
            vec4 sampledColor = texture(texTransfunc, scalar.r);
            color = color + sampledColor * vec4(sampledColor.aaa, 1.0) * (1.0 - color.a);
            if (color.a > 0.95) {
                break;
            }
        } else {
            if (sign(scalar.r - isovalue) != sign(prev_value - isovalue)) {
                color = vec4(isosurface_color, 1.0);
                break;
            }
            prev_value = scalar.r;
        }
    }
    color = color + vec4(bg.rgb, 0.0) * (1.0 - color.a);
    color.a = 1.0;
    fragColor = color;
}
