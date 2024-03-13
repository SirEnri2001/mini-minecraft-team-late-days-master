#version 420
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture;
uniform int u_Time;
uniform vec3 cameraposition;
layout(binding = 2) uniform sampler2D u_shadowMap;
// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_EyeVec;
in vec4 fs_Col;
in vec4 fs_shadowCoord;

in vec4 fs_debugCol;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

const float fogstart=100;
const float fogend=250;
const vec4 fogcolor=vec4(0.5,0.5,0.5, 1);

float computeFogFactor(float distance) {
    float fogFactor;
    if(distance<fogstart){
        fogFactor= 0.;
    } else{
        fogFactor= 1 - (fogend - distance) / (fogend - fogstart);
        //clamp(fogFactor, 0.0, 1.1);
    }
    return fogFactor;
}
float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

//using uv to test if it is lava or water
bool animationornot(vec2 textureuv){
    if(textureuv.x>=13.0/16.0&&textureuv.y<=4.0/16.0){
        return true;
    }
    else{
        return false;
    }
}

bool SHADOWZ = true;
void main()
{
//    if (SHADOWZ) {
//        out_Col = vec4(vec3(texture(u_shadowMap, fs_shadowCoord.xy).r), 1);
//    } else {
//        out_Col = vec4(vec3(fs_shadowCoord.z), 1);
//    }
//    return;
    vec2 uv=vec2(fs_Col.x,fs_Col.y);
    // Material base color (before shading)
   if(uv.x>=13.0/16.0&&uv.y<=4.0/16.0&&uv.y>=3.0/16.0){
       vec2 status=vec2(fs_Col.z,fs_Col.w);
       if(status.x==1.0/16.0){
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==1.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
       }
       else if(status.y==2.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
       }
       else if(status.y==3.0/16.0){
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==4.0/16.0){
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==5.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==6.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==7.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }
       else if(status.y==8.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=3.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+3.0)/16.0;
       }

       else{
        uv.x*=16.0;
        uv.x-=13.0;
        uv.x=fract(uv.x+u_Time/100.f);
        uv.x=(uv.x+13.0)/16.0;}
    }

   if(uv.x>=13.0/16.0&&uv.y<=2.0/16.0&&uv.y>=1.0/16.0){
       vec2 status=vec2(fs_Col.z,fs_Col.w);
       if(status.x==1.0/16.0){
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==1.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
       }
       else if(status.y==2.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
       }
       else if(status.y==3.0/16.0){
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==4.0/16.0){
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==5.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==6.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x-u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==7.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y+u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }
       else if(status.y==8.0/16.0){
           uv.x*=16.0;
           uv.x-=13.0;
           uv.x=fract(uv.x+u_Time/100.f);
           uv.x=(uv.x+13.0)/16.0;
           uv.y*=16.0;
           uv.y-=1.0;
           uv.y=fract(uv.y-u_Time/100.f);
           uv.y=(uv.y+1.0)/16.0;
       }

       else{
        uv.x*=16.0;
        uv.x-=13.0;
        uv.x=fract(uv.x+u_Time/100.f);
        uv.x=(uv.x+13.0)/16.0;
       }
    }

   // lambert
   float ambientTerm = 0.2;

        //vec4 diffuseColor = fs_Col;
        float dist=length(fs_Pos.xyz-cameraposition);
        float fogfactor=1;
        vec4 diffuseColor=texture(u_Texture,uv);
        float alpha=diffuseColor.a;
        diffuseColor = diffuseColor;

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float specularTerm = max(0, dot(normalize(fs_Nor), (normalize(fs_EyeVec) + normalize(fs_LightVec))/2) * 0.2);

    // Calculate shadow
    float shadow = 0;
    float bias = max(0.02 * (1.0 - dot(fs_Nor, fs_LightVec)), 0.005);
    vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
    // PCF (percentage-closer filtering)
    int sampleCount = 0;
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(u_shadowMap, fs_shadowCoord.xy  + vec2(x, y) * texelSize).r;
            shadow += fs_shadowCoord.z - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount += 1;
        }
    }
    shadow /= 2 * sampleCount - 2;

    float sunFactor = dot(normalize(fs_LightVec), vec4(0,1,0, 0));
    if (sunFactor < -0.2) {
        diffuseTerm *= 1 + sunFactor;
        specularTerm = 0;
    }

    float lightIntensity = (1 - shadow) * diffuseTerm +  ((1 - shadow)*0.5 + 0.1) * specularTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    vec4 sceneColor = vec4(diffuseColor.rgb * lightIntensity, alpha);

    if (computeFogFactor(length((fs_Pos.xyz - cameraposition.xyz))) >= 1.2) {
        out_Col = vec4(0);
        return;
    }
    sceneColor = mix(sceneColor, fogcolor, computeFogFactor(length((fs_Pos.xyz - cameraposition.xyz))));

    // Compute final shaded color
    out_Col = sceneColor;
    //out_Col = vec4(vec3((1 - shadow)), 1);
    //gl_FragDepth = texture(u_shadowDepth, fs_shadowCoord.xy).x;

}
