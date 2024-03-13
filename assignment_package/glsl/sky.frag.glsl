#version 330

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

//uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;
uniform int u_Weather;
in vec4 fs_Pos;
in vec2 fs_UV;
in vec4 fs_Col;
out vec4 out_Col;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(255, 137, 103) / 255.0,
                               vec3(253, 96, 81) / 255.0,
                               vec3(57, 32, 51) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
                            vec3(96, 72, 120) / 255.0,
                            vec3(72, 48, 120) / 255.0,
                            vec3(48, 24, 96) / 255.0,
                            vec3(0, 24, 72) / 255.0);

const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 cloudColor = sunset[3];

float random1( vec3 p ) {
    return fract(sin((dot(p, vec3(127.1,
                                  311.7,
                                  191.999)))) *
                 43758.5453);
}
vec2 smoothF(vec2 uv)
{
    return uv*uv*(3.f-2.f*uv);
}
float noise(vec2 uv)
{
    const float k = 257.;
    vec2 fl = vec2(floor(uv[0]),floor(uv[1]));
    vec2 fr = vec2(fract(uv[0]),fract(uv[1]));
    vec4 l  = vec4(fl[0],fl[1],fr[0],fr[1]);
    float u = l.x + l.y * k;
    vec4 v  = vec4(u, u+1.,u+k, u+k+1.);
    v       = fract(fract(1.23456789f*v)*v/.987654321f);
    vec2 zw    = smoothF(vec2(l[2],l[3]));
    l[2] = zw[0];
    l[3] = zw[1];
    l.x     = mix(v.x, v.y, l.z);
    l.y     = mix(v.z, v.w, l.z);
    return    mix(l.x, l.y, l.w);
}

float interpNoise3D(vec3 uvw) {
    float x = uvw.x;
    float y = uvw.y;
    float z = uvw.z;
    int intX = int(floor(x));
    float fractX = fract(x);
    int intY = int(floor(y));
    float fractY = fract(y);
    int intZ = int(floor(z));
    float fractZ = fract(z);

    float v1 = random1(vec3(intX, intY, intZ));
    float v2 = random1(vec3(intX + 1, intY, intZ));
    float v3 = random1(vec3(intX, intY + 1, intZ));
    float v4 = random1(vec3(intX + 1, intY + 1, intZ));
    float v5 = random1(vec3(intX, intY, intZ+1));
    float v6 = random1(vec3(intX + 1, intY, intZ+1));
    float v7 = random1(vec3(intX, intY + 1, intZ+1));
    float v8 = random1(vec3(intX + 1, intY + 1, intZ+1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);
    float i3 = mix(v5,v6,fractX);
    float i4 = mix(v7,v8,fractX);

    float j1 = mix(i1,i2,fractY);
    float j2 = mix(i3,i4,fractY);
    return mix(j1, j2, fractZ);
}


float FBM3d(vec3 uvw) {
    float a = 0.5;
    float n = 0.;
    float f = 1.0;
    int it = 4;
    mat3 rotMat = mat3(1,0,0,
        0,4.0/5.0,-3.0/5.0,
        0,4.0/5.0,3.0/5.0
    );
    for(int i = 0; i < it; i++)
    {
        n += interpNoise3D(rotMat*uvw*f)*a;

        a *= 0.5;
        f *= 2.;
    }
    return n;
}

float FBM(vec2 uv) {
    float a = 0.5;
    float f = 5.0;
    float n = 0.;
    int it = 8;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            n += noise(uv*f)*a;
            a *= .5;
            f *= 2.;
        }
    }
    return n;
}

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.5) {
        return dusk[0];
    }
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    return dusk[4];
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}

float surflet(vec3 p, vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    vec3 t2 = abs(p - gridPoint);
    vec3 t = vec3(1.f) - 6.f * pow(t2, vec3(5.f)) + 15.f * pow(t2, vec3(4.f)) - 10.f * pow(t2, vec3(3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    vec3 gradient = random3(gridPoint).x * vec3(2.) - vec3(1., 1., 1.);
    // Get the vector from the grid point to P
    vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}

float PerlinNoise3d(vec3 uvw){
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet(uvw, floor(uvw) + vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;

}

vec3 PerlinNoise3dDerivUW(vec3 uvw){
    float p = PerlinNoise3d(uvw);
    float p_delU = PerlinNoise3d(uvw+vec3(0.001,0,0));
    float p_delW = PerlinNoise3d(uvw+vec3(0,0,0.001));
    return vec3(p_delU-p,0,p_delW-p);
}

float getCloud(vec3 rayDir){
    mat3 scale = mat3(
        1,0,0,
        0,1,0,
        0,0,smoothstep(-1.0,1.0,WorleyNoise(rayDir.xz/2))
    );
    return smoothstep(0.2,0.8,FBM3d(scale*rayDir*10))*smoothstep(-0.5,0.2,PerlinNoise3d(scale*rayDir*2));
}

#define RAY_AS_COLOR
//#define SPHERE_UV_AS_COLOR
//#define WORLEY_OFFSET

void main()
{

    vec3 outCol;
    vec2 ndc = (fs_UV) * 2.0 - 1.0; // -1 to 1 NDC
    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);

    vec3 earthCenter = vec3(0.f,-8.f,0.f);

    float scaleRatio =
            (-2*rayDir.y*(-earthCenter).y+sqrt(pow(-2*rayDir.y*(-earthCenter).y,2.0)+4*(pow(1-earthCenter.y,2)-earthCenter.y*earthCenter.y)))/2;
    vec3 skyDome = rayDir*scaleRatio-earthCenter;
    vec2 uv = sphereToUV(skyDome);
    vec3 worldUp = vec3(0,1,0);
    vec3 bgClearColor = vec3(74.0, 154.0, 189.0)/256.0;
    if(u_Weather!=0){
        bgClearColor = vec3(0.5,0.5,0.5);
    }
    vec3 bgNightClearColor = vec3(15, 13, 46)/256.0;
    float cloudCoeff = getCloud(skyDome)*dot(worldUp,rayDir);
    mat3 sunRot = mat3(
        1,0,0,
        0,cos(u_Time/100.0),-sin(u_Time/100.0),
        0,sin(u_Time/100.0),cos(u_Time/100.0)
                );
    vec3 sunDir = sunRot*normalize(vec3(0.0,0.0,-1.0));
    float dayNightCycle = dot(worldUp, sunDir);
    float sunScale = dot(worldUp, sunDir+vec3(0,0.2,0));
    vec3 dayCloudColor = vec3(1.0);
    vec3 nightCloudColor = vec3(0.5);
    vec3 cloudVisColor = vec3(1.0);
    vec3 bgColor = mix(bgNightClearColor,bgClearColor,(dayNightCycle+1.0)/2);

    vec3 starColor = vec3(smoothstep(0.6,0.7,(1-WorleyNoise3D(rayDir*50))*(1+PerlinNoise3d(rayDir*10))/2.0));
    bgNightClearColor+=starColor;
    float rayFromSun = dot(rayDir,sunDir);
    if(dayNightCycle>0.5){
        bgColor = bgClearColor;
        cloudVisColor = dayCloudColor;
    }else if(dayNightCycle<-0.5){
        bgColor = bgNightClearColor;
        cloudVisColor = nightCloudColor;
    }else{
        if(rayFromSun>0.995){
            cloudVisColor = mix(sunset[1],sunset[0],(rayFromSun-0.995)/0.005);
            bgColor = mix(dusk[1],dusk[0],(rayFromSun-0.995)/0.005);
        }else if(rayFromSun>0.95){
            cloudVisColor = mix(sunset[2],sunset[1],(rayFromSun-0.95)/0.045);
            bgColor = mix(dusk[2],dusk[1],(rayFromSun-0.95)/0.045);
        }else if(rayFromSun>0.7){
            cloudVisColor = mix(sunset[3],sunset[2],(rayFromSun-0.7)/0.25);
            bgColor = mix(dusk[3],dusk[2],(rayFromSun-0.7)/0.25);
        }else if(rayFromSun>0.2){
            cloudVisColor = mix(sunset[4],sunset[3],(rayFromSun-0.2)/0.5);
            bgColor = mix(dusk[4],dusk[3],(rayFromSun-0.2)/0.5);
        }else if(rayFromSun>-0.8){
            cloudVisColor = mix(vec3(1.0),sunset[4],(rayFromSun+0.8));
            bgColor = mix(bgNightClearColor,dusk[4],(rayFromSun+0.8));
        }else{
            bgColor = bgNightClearColor;
        }

        if(dayNightCycle>0.0){
            bgColor = mix(bgColor,bgClearColor,dayNightCycle/0.5);
            cloudVisColor = mix(cloudVisColor,dayCloudColor,dayNightCycle/0.5);

        }else{
            bgColor = mix(bgColor,bgNightClearColor,-dayNightCycle/0.5);
            cloudVisColor = mix(cloudVisColor,nightCloudColor,-dayNightCycle/0.5);
        }
    }



    vec3 sunVisColor = max(0,(smoothstep(0.9,0.97,pow(dot(sunDir,rayDir),mix(10,500,(abs(sunScale)))))-cloudCoeff))*sunColor;
    vec3 color = mix(bgColor,cloudVisColor,cloudCoeff)+sunVisColor;
    //color = bgNightClearColor;
    out_Col = vec4(color,1);
}
