#version 430 core

in vec4 rayCastDir;//暂时不需要用到

out vec4 fragColor;

layout(binding=0,r32ui) uniform uimage3D blocks;//read-only 废弃了 不需要

layout(binding=1,rgba32f) uniform image2D entryPos;
layout(binding=2,rgba32f) uniform image2D exitPos;

uniform sampler1D transferFunc;
uniform sampler2D preIntTransferFunc;
uniform sampler3D volumeTexture0;
uniform sampler3D volumeTexture1;
uniform sampler3D volumeTexture2;
uniform sampler3D volumeTexture3;

layout(std430,binding=0) buffer PageTable{
    uvec4 pageEntry[];
}pageTable;

//debug use
layout(std430,binding=1) buffer Test{
    vec4 virtualIndex[];
}test;

uniform float lengthPerBlock;// = 1.0/maxBlockDim
uniform int blockLength;
uniform int padding;
uniform ivec3 blockDim;//虚拟数据块的dim
uniform int maxBlockDim;
uniform float step;//step=1.0/maxBlockSize*0.3 (<0.5 is ok)
//const float step=1.0/256*0.3;

uniform ivec3 textureSize;//一个数据块纹理的大小，用于计算采样点在纹理中的真正坐标


uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 lightDirection;//平行环境光

uniform bool usePreIntTF;
uniform int drawMode;//0 for mip, 1 for iosFace, 2 for normal tf draw
uniform float isoValue;
uniform vec3 isoColor;

uniform vec3 bgColor;//should set black color while draw mip
//uniform vec3 boundRatio;//用于非立方体数据 不需要
uniform mat4 model;

vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 Normal)
{
    vec3 N=Normal;
    N=N*2.0-1.0;
    N=(model*vec4(N,0.0f)).xyz;
    N=-normalize(N);

    vec3 L=-normalize(lightDirection);
    vec3 R=L;

    vec3 ambient=ka*diffuseColor.rgb;
    vec3 specular=ks*pow(max(dot(N,(L+R)/2.0),0.0),shininess)*vec3(1.0,1.0,1.0);
    vec3 diffuse=kd*max(dot(N,L),0.0)*diffuseColor.rgb;
    return ambient+specular+diffuse;
}

vec4 virtualSample(vec3 samplePos)
{
    vec4 scalar;

    vec3 tmp=samplePos*maxBlockDim-vec3(0.00001);//float精度问题
    ivec3 virtualIdx=ivec3(tmp);

    //采样点在虚拟块中的偏移量
    vec3 offsetInBlock=(samplePos-virtualIdx*lengthPerBlock)/lengthPerBlock*(blockLength-2*padding);
    int virtualFlatIdx=virtualIdx.z*blockDim.y*blockDim.x+virtualIdx.y*blockDim.x+virtualIdx.x;

    //获得数据块在显存中的物理索引，通过物理索引计算在纹理中的实际坐标
    uvec4 physicalIdx=pageTable.pageEntry[virtualFlatIdx];
    uint texId=physicalIdx.w;
    vec3 samplePoint=(vec3(physicalIdx.xyz*blockLength)+offsetInBlock+vec3(padding))/vec3(textureSize);

    //debug use
    //int idx=int(floor(gl_FragCoord.y))*1200+int(floor(gl_FragCoord.x));
    //test.virtualIndex[idx]=vec4(samplePoint,1.1);

    //rgb represent gradient, a represent real scalar
    if(texId==0){
        scalar=texture(volumeTexture0,samplePoint);
    }
    else if(texId==1){
        scalar=texture(volumeTexture1,samplePoint);
    }
    else if(texId==2){
        scalar=texture(volumeTexture2,samplePoint);
    }
    else if(texId==3){
        scalar=texture(volumeTexture3,samplePoint);
    }
    else{
        scalar=vec4(0.0,0.0,0.0,0.0);
    }
    return scalar;
}

void main()
{
    vec3 startPos=imageLoad(entryPos, ivec2(gl_FragCoord.xy)).xyz;
    vec3 endPos=imageLoad(exitPos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 start2end=endPos-startPos;
    vec3 rayDirection=normalize(start2end);

    float distance=dot(start2end,rayDirection);
    int steps=int(distance/step);

    vec4 color=vec4(0.0,0.0,0.0,0.0);
    vec3 samplePos=startPos;
    vec4 oldScalar=virtualSample(samplePos);
    int i;
    if(drawMode==0){
        vec4 scalar=vec4(0.0,0.0,0.0,0.0);
        for(i=0;i<steps;i++){
            vec4 tmp=virtualSample(samplePos);
            if(tmp.a>scalar.a)
                scalar=vec4(tmp.a,tmp.a,tmp.a,tmp.a);

            samplePos += rayDirection*step;

        }
        color=scalar;
    }
    else if(drawMode==1){

        float lastScalar=virtualSample(samplePos).a;

        for(i=0;i<steps;i++){
            vec4 tmp=virtualSample(samplePos);
            if(lastScalar<=isoValue && isoValue<=tmp.a){
                color=vec4(isoColor,1.0);
                color.rgb=phongShading(samplePos,color.rgb,tmp.rgb);
                break;
            }
            lastScalar=tmp.a;
            samplePos += rayDirection*step;
        }
    }
    else if(drawMode==2){
        for(i=0;i<steps;i++){
            vec4 scalar=virtualSample(samplePos);
            vec4 sampleColor;
            if(usePreIntTF)
                sampleColor=texture(preIntTransferFunc,vec2(oldScalar.a,scalar.a));
            else
                sampleColor=texture(transferFunc,scalar.a);

            sampleColor.rgb=phongShading(samplePos,sampleColor.rgb,scalar.rgb);
            color = color + sampleColor * vec4( sampleColor.aaa, 1.0 ) * ( 1.0 - color.a );

            if(color.a>0.99)
            break;

            oldScalar=scalar;
            samplePos+=rayDirection*step;
        }
    }

    //debug use
    //int idx=int(floor(gl_FragCoord.y))*1200+int(floor(gl_FragCoord.x));
    //test.virtualIndex[idx]=vec4(samplePos,float(i)/steps);

    if(color.a==0.0) discard;
    if(drawMode==0)//mip
        color=color+vec4(0.0,0.0,0.0,0.0)*(1.0-color.a);
    else
        color = color + vec4( bgColor.rgb, 0.0 ) * ( 1.0 - color.a );
    color.a=1.0;
    fragColor=color;
}

