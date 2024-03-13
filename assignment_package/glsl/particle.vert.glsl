#version 330 core
layout (location = 0) in vec4 vs_Pos;

uniform mat4 u_Model;
uniform mat4 u_ViewProj;
uniform vec3 u_Eye;
uniform int u_Time;
uniform int u_Weather;

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

void main()
{
    float speed = 1.0/20.0;
    if(u_Weather==2){
        speed = 1.0/60.0;
    }
    vec3 basePoint = floor(u_Eye)-vec3(2.0,0.5,2.0);
    vec4 rainDropPos = vec4(4*random3(vec3(vs_Pos.xyz)),1.0);
    rainDropPos.y+= -u_Time*speed;
    rainDropPos.y = mod(floor(rainDropPos.y), 4)+basePoint.y+fract(rainDropPos.y);
    rainDropPos.x = mod(floor(rainDropPos.x), 4)+basePoint.x+fract(rainDropPos.x);
    rainDropPos.z = mod(floor(rainDropPos.z), 4)+basePoint.z+fract(rainDropPos.z);
//    rainDropPos.y+=300;
    gl_Position = rainDropPos;
}
