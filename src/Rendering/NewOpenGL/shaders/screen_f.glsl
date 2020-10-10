#version 430 core
in vec2 screenTexCoord;
out vec4 fragColor;
layout(location=0) uniform sampler1D TF;
layout(location=1) uniform sampler3D Block;
layout(location=2) uniform sampler2D preIntTF;
layout(location=3) uniform sampler2D entryPosTexture;
layout(location=4) uniform sampler2D exitPosTexture;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 lightDir;
const float step=0.001;
uniform bool usePreIntTF;
uniform bool gradPreCal;
uniform bool isMipMap;
uniform vec3 bgColor;
vec3 rayDirection;
//const vec3 bg=vec3(1.0,1.0,1.0);
vec3 phongShading(vec3 samplePos,vec3 diffuseColor)
{
    vec3 N;
    if(!gradPreCal){
        N.x=(texture(Block,samplePos+vec3(step,0,0)).r-texture(Block,samplePos+vec3(-step,0,0)).r);
        N.y=(texture(Block,samplePos+vec3(0,step,0)).r-texture(Block,samplePos+vec3(0,-step,0)).r);
        N.z=(texture(Block,samplePos+vec3(0,0,step)).r-texture(Block,samplePos+vec3(0,0,-step)).r);
    }
    else{
        N=texture(Block,samplePos).xyz;
        N=N*2.0-1.0;
    }
    N=-normalize(N);
    vec3 L=-normalize(lightDir);
    vec3 R=-rayDirection;

    vec3 ambient=ka*diffuseColor.rgb;
    vec3 specular=ks*pow(max(dot(N,(L+R)/2.0),0.0),shininess)*vec3(1.0,1.0,1.0);
    vec3 diffuse=kd*max(dot(N,L),0.0)*diffuseColor.rgb;

    return ambient+specular+diffuse;

}
vec4 calColor(vec2 texCoord)
{
    vec3 startPos=texture(entryPosTexture,texCoord).xyz;
    vec3 exitPos=texture(exitPosTexture,texCoord).xyz;
    vec3 start2exit=exitPos-startPos;

//    if (start2exit.x == 0.0 && start2exit.y == 0.0 && start2exit.z == 0.0) {
//        gl_FragColor = bg; // Background Colors
//        return vec4(0.0,0.0,0.0,0.0);
//    }
    rayDirection=normalize(start2exit);
   // return vec4(startPos,1.0);
    vec4 color=vec4(0.0, 0.0, 0.0, 0.0);
    float distance=dot(start2exit,rayDirection);
    int steps=int (distance/step);

    //vec4 oldScalar=texture(Block,startPos);

    int i;
    //highp vec3 N;
    //    if(isMipMap){
    //        for(int i=0;i<steps;i++){
    //            vec4 tmp=texture(Block,startPos);
    //            if(gradPreCal){
    //                if(tmp.a>scalar.a)
    //                scalar=vec4(tmp.a,tmp.a,tmp.a,tmp.a);
    //            }
    //            else{
    //                if(tmp.r>scalar.r)
    //                scalar=vec4(tmp.r,tmp.r,tmp.r,tmp.r);
    //            }
    //            startPos += rayDirection*step;
    //        }
    //        color=scalar;
    //    }
    //    else{
    for ( i = 0; i < steps; ++i ){
        //   if(startPos.x<-0.1 || startPos.x>1.1 || startPos.y<-0.1 ||startPos.y>1.1 || startPos.z<-0.1 || startPos.z>1.1)
        //        if(startPos.x<0.0 || startPos.x>1.0 || startPos.y<0.0 ||startPos.y>1.0 || startPos.z<0.0 || startPos.z>1.0)
        //               break;
        vec3 samplePos = startPos+rayDirection*step*(float(i)+0.5);
        vec4 scalar=texture(Block, samplePos);
        //            vec4 newScalar=texture(Block,startPos+rayDirection*step);
        //            if(!gradPreCal){
        //                if(usePreIntTF){
        //                    //               sampleColor=texture(TF,scalar.r);
        //                    sampleColor=texture(preIntTF,vec2(newScalar.r,scalar.r));
        //                }
        //                else{
        //                    sampleColor=texture(TF,scalar.r);
        //                }
        //            }
        //            else{
        //                if(usePreIntTF){
        //                    //sampleColor=texture(TF,scalar.a);
        //                    //    sampleColor=texture(preIntTF,vec2(newScalar.a,scalar.a));
        //                    sampleColor=texture(preIntTF,vec2(oldScalar.a,scalar.a));
        //                }
        //                else{
        vec4 sampleColor=texture(TF,scalar.a);
        //                }
        //            }
        //sampleColor=texture(TF,scalar.a);
        sampleColor.rgb=phongShading(samplePos,sampleColor.rgb);
        color = color + sampleColor * vec4( sampleColor.aaa, 1.0 ) * ( 1.0 - color.a );

        if ( color.a > 0.99 )
        break;

        //oldScalar=scalar;
    }
    //    }
    return color;
}
void main() {
    vec4 color=calColor(screenTexCoord);
//    if(color.a==0.0) discard;
//
////    color = color + vec4( bg, 0.0 ) * ( 1.0 - color.a );
////    color.a=1.0;
////    fragColor=color;
////    //fragColor=vec4(screenTexCoord,0.0,0.0);
////  //  fragColor=vec4(1.0,1.0,0.0,0.0);
//    vec3 rayStart = texture(entryPosTexture, screenTexCoord).xyz;
////    fragColor = vec4(rayStart,1.0);
////    return;
//    vec3 rayEnd = texture(exitPosTexture, screenTexCoord).xyz;
//    vec3 start2end = rayEnd - rayStart;
//    //    vec4 bg = vec4(0.156863, 0.156863, 0.156863, 1.0);
   vec4 bg = vec4(1.0, 1.0, 1.0, 1.0);
//
//    if (start2end.x == 0.0 && start2end.y == 0.0 && start2end.z == 0.0) {
//        fragColor = bg; // Background Colors
//        return;
//    }
//    vec4 color = vec4(0, 0, 0, 0);
//    vec3 direction = normalize(start2end);
//    float distance = dot(direction, start2end);
//    int steps = int(distance / step);
//    for(int i = 0; i < steps; ++i) {
//
//        vec3 samplePoint  = rayStart + direction * step * (float(i) + 0.5);
//        vec4 scalar = texture(Block, samplePoint).xyzw;
//        vec4 sampledColor = texture(TF, scalar.a);
//        //sampledColor.rgb = phongShading(samplePoint, sampledColor.rgb);
//
//        color = color + sampledColor * vec4(sampledColor.aaa, 1.0) * (1.0 - color.a);
//        if(color.a > 0.99)
//        break;
//    }
    if(color.a == 0.0) discard;
    color = color + vec4(bg.rgb, 0.0) * (1.0 - color.a);
    color.a = 1.0;
    fragColor = color;
}