#pragma once

namespace shader {

const char *position_f = R"(#version 430 core
in vec3 texCoord;

// Multi Render Targets
layout(location = 0) out vec4 entryPos;
layout(location = 1) out vec4 exitPos;

layout(location = 1) uniform mat4 ModelMatrix;
layout(location = 2) uniform vec3 viewPos;


/*
* State configuration:
* 1.Enable blending
* 2.Set blend function as srcRGB *(1), dstRBG(1), srcAlpha(1), dstAlpha(1)
* 3.Set the polygon front face according to your vertex orderer.
*/
//out vec4 fragColor;
void main()
{
    vec3 maxPoint = vec3(ModelMatrix*vec4(1, 1, 1, 1));
    vec3 minPoint = vec3(ModelMatrix*vec4(0, 0, 0, 1));

    bool inner = false;
    vec3 eyePos = viewPos;
    if (eyePos.x >= minPoint.x && eyePos.x <= maxPoint.x
        && eyePos.y >= minPoint.y && eyePos.y <= maxPoint.y
        && eyePos.z >= minPoint.z && eyePos.z <= maxPoint.z) {
        inner = true;
    }

    if (gl_FrontFacing) {
        entryPos = vec4(texCoord, 1.0);
        exitPos = vec4(0, 0, 0, 0);
    }
    else
    {
        exitPos = vec4(texCoord, 1.0);
        if (inner){
            eyePos = (eyePos-minPoint)/(maxPoint-minPoint);// normalized to sample space of [0,1]^3
            entryPos=vec4(eyePos, 0);
        } else {
            entryPos=vec4(0, 0, 0, 0);
        }
    }
    return;
}
)";

const char *position_v = R"(#version 430 core
layout(location = 0) uniform mat4 MVPMatrix;

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 TexturePosition;

out vec3 texCoord;

void main()
{
	gl_Position = MVPMatrix * vec4(VertexPosition,1.0);
	texCoord = TexturePosition; //(0,0,0)<--->(1,1,1)
})";

const char *screenquad_v = R"(#version 430 core
const vec2 screenQuadVertexCoord[4]={vec2(-1.0,-1.0),vec2(1.0,-1.0),vec2(-1.0,1.0),vec2(1.0,1.0)};
void main()
{
    gl_Position = vec4(screenQuadVertexCoord[gl_VertexID].x,screenQuadVertexCoord[gl_VertexID].y,0.0,1.0);
})";

const char *naiveraycast_f = R"(#version 430 core

layout(location = 0,rgba32f) uniform volatile image2D entryPosTexture;
layout(location = 1,rgba32f) uniform volatile image2D exitPosTexture;
layout(location = 2,rgba32f) uniform volatile image2D resutlTexture;
layout(location = 3) uniform sampler1D texTransfunc;
layout(location = 4) uniform sampler3D volumeTexture;

in vec2 screenCoord;
out vec4 fragColor;
float step = 0.01;

void main()
{
	vec3 rayStart = vec3(imageLoad(entryPosTexture,ivec2(gl_FragCoord)).xyz);
	vec3 rayEnd = vec3(imageLoad(exitPosTexture,ivec2(gl_FragCoord)).xyz);

	vec3 start2end = vec3(rayEnd - rayStart);
	vec4 color = imageLoad( resutlTexture, ivec2( gl_FragCoord ) ).xyzw;
	//vec4 color = vec4(0);
	vec4 bg = vec4( 0.f, 0.f, 0.f, .00f );
	vec3 direction = normalize( start2end );
	float distance = dot( direction, start2end );
	int steps = int( distance / step );
	vec3 samplePoint = rayStart;

	for ( int i = 0; i < steps; ++i ) {
		samplePoint += direction * step;
		vec4 scalar = texture(volumeTexture,samplePoint);
		vec4 sampledColor = texture(texTransfunc, scalar.r);
		//sampledColor.a = 1-pow((1-sampledColor.a),correlation[curLod]);
		color = color + sampledColor * vec4( sampledColor.aaa, 1.0 ) * ( 1.0 - color.a );
		if ( color.a > 0.99 ) {
			break;
		}
	}
	color = color + vec4( bg.rgb, 0.0 ) * ( 1.0 - color.a );
	color.a = 1.0;
	fragColor = color;
}
)";

const char *screenquad_f = R"(#version 430 core
layout(location = 0,rgba32f) uniform volatile image2D screenQuadTexture;
out vec4 fragColor;

void main()
{
    fragColor = imageLoad(screenQuadTexture,ivec2(gl_FragCoord)).xyzw;
})";

} // namespace shader