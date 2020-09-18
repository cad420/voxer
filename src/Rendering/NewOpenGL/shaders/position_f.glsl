#version 430 core
in vec3 texCoord;
out vec4 fragColor;
void main()
{
 //   fragColor=vec4(1.0,0.0,0.0,1.0);
    fragColor=vec4(texCoord,1.0);
}
//in vec3 texCoord;
////Multi Render Targets
//layout(location=0) out vec4 entryPos;//location 0 for COLOR_ATTACHMENT0
//layout(location=1) out vec4 exitPos;
//
////uniform mat4 ModelMatrix;
//uniform vec3 viewPos;
//
///*
//* State configuration:
//* 1.Enable blending
//* 2.Set blend function as srcRGB *(1), dstRBG(1), srcAlpha(1), dstAlpha(1)
//* 3.Set the polygon front face according to your vertex orderer.
//*/
//void main()
//{
//    vec3 maxPoint=vec3(1.0);
//    vec3 minPoint=vec3(0.0);
//    bool inner=false;
//    vec3 eyePos=viewPos;
//    if(eyePos.x>=minPoint.x && eyePos.x<=maxPoint.x
//    && eyePos.y>=minPoint.y && eyePos.y<=maxPoint.y
//    && eyePos.z>=minPoint.z && eyePos.z<=maxPoint.z)
//        inner=true;
//
//    if(gl_FrontFacing){
//        entryPos=vec4(texCoord,1.0);
//        exitPos=vec4(0.0);
//    }
//    else{
//        exitPos=vec4(texCoord,1.0);
//        if(inner){
//            eyePos=(eyePos-minPoint)/(maxPoint-minPoint);
//            entryPos=vec4(eyePos,0);
//        }
//        else{
//            entryPos=vec4(0.0);
//        }
//    }
//    return;
//}
