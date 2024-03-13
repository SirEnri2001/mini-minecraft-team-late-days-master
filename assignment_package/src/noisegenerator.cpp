#include "noisegenerator.h"
#include <QDebug>
using namespace glm;
vec2 random2( vec2 p ) {
    return normalize(2.f * fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453f) - 1.f);
}
float smoothstep(float edge0, float edge1, float x) {
    // Scale, bias and saturate x to 0..1 range
    x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}
float NoiseGenerator::WorleyNoise(glm::vec2 uv){
    using namespace glm;
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.
    float secondMinDist = 1.0;
    vec2 closestPoint;

    // Search all neighboring cells and this cell for their point
    for(int y = -2; y <= 2; y++)
    {
        for(int x = -2; x <= 2; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            if(dist < minDist) {
                secondMinDist = minDist;
                minDist = dist;
                closestPoint = point;
            }
            else if(dist < secondMinDist) {
                secondMinDist = dist;
            }
        }
    }
    float height = 0.5 * minDist + 0.5 * secondMinDist;
    //height = length(closestPoint);
    //    height = height * height;
    return height;
}
float surflet(vec2 P, vec2 gridPoint)
{
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);

    // Get the random vector for the grid point
    vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}
vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7,114)),
                          dot(p,vec3(269.5, 183.3,514)),
                          dot(p, vec3(420.6, 631.2,679))
                          )) * vec3(43758.5453));
}

float surflet(vec3 p, vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    vec3 t2 = abs(p - gridPoint);
    vec3 t = vec3(1.f) - 6.f * pow(t2, vec3(5.f)) + 15.f * pow(t2, vec3(4.f)) - 10.f * pow(t2, vec3(3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    vec3 gradient = random3(gridPoint) * vec3(2.) - vec3(1., 1., 1.);
    // Get the vector from the grid point to P
    vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}

float NoiseGenerator::PerlinNoise3d(glm::vec3 uvw){
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

float NoiseGenerator::PerlinNoise(glm::vec2 uv){    // Tile the space
    using namespace glm;
    vec2 uvXLYL = floor(uv);
    vec2 uvXHYL = uvXLYL + vec2(1,0);
    vec2 uvXHYH = uvXLYL + vec2(1,1);
    vec2 uvXLYH = uvXLYL + vec2(0,1);

    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}

glm::vec2 smoothF(glm::vec2 uv)
{
    return uv*uv*(3.f-2.f*uv);
}
float noise(glm::vec2 uv)
{
    const float k = 257.;
    glm::vec2 fl = glm::vec2(glm::floor(uv[0]),glm::floor(uv[1]));
    glm::vec2 fr = glm::vec2(glm::fract(uv[0]),glm::fract(uv[1]));
    glm::vec4 l  = glm::vec4(fl[0],fl[1],fr[0],fr[1]);
    float u = l.x + l.y * k;
    glm::vec4 v  = glm::vec4(u, u+1.,u+k, u+k+1.);
    v       = glm::fract(glm::fract(1.23456789f*v)*v/.987654321f);
    glm::vec2 zw    = smoothF(glm::vec2(l[2],l[3]));
    l[2] = zw[0];
    l[3] = zw[1];
    l.x     = glm::mix(v.x, v.y, l.z);
    l.y     = glm::mix(v.z, v.w, l.z);
    return    glm::mix(l.x, l.y, l.w);
}

float NoiseGenerator::FBM(glm::vec2 uv) {
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
float random1( vec3 p ) {
    return fract(sin((dot(p, vec3(127.1,
                                  311.7,
                                  191.999)))) *
                 43758.5453);
}

float NoiseGenerator::FBM3d(glm::vec3 uvw) {
    float a = 0.5;
    float f = 1.0;
    float n = 0.;
    int it = 2;
    for(int i = 0; i < it; i++)
    {
        n += random1(uvw*f)*a;
        a *= .5;
        f *= 2.;
    }
    return n;
}

float NoiseGenerator::generateCave(glm::vec3 uvw){  // gives out 0 to 1, greater than 0.5 should generate cave
    if (uvw.y>127){
        return 0;
    }
    float freq = 32.f;
    //glm::vec3 offset = glm::vec3(FBM3d(uvw/fbmFreq),  FBM3d(uvw/fbmFreq+glm::vec3(100,150,75)),FBM3d(uvw/fbmFreq+glm::vec3(10,190,375)));
    float h = PerlinNoise3d(uvw/freq);
    float offset = 0.1*PerlinNoise(glm::vec2(uvw.x+114,uvw.z+514)/4.f);
    float heightIndex = pow(1-uvw.y/128,0.3);
    h+=offset;
    h = (-h+1)/2/1.15;
    h*=heightIndex;
    return h;
}

float NoiseGenerator::generateMountain(glm::vec2 uv)
{
    glm::vec2 xz = uv;
    float freq = 32;
    glm::vec2 offset = glm::vec2(FBM(xz/freq)*2-1,  FBM(xz/freq+glm::vec2(100,150))*2-1);
    float h = WorleyNoise(xz/freq+offset*0.2f);
    return floor(129+h * 50);
}

float NoiseGenerator::generatePlain(glm::vec2 uv)
{
    glm::vec2 xz = uv;

    float h = 0;

    float amp = 0.1;
    float freq = 128;
    for(int i = 0; i < 4; ++i) {
        glm::vec2 offset = glm::vec2(FBM(xz / 256.f), FBM(xz / 300.f));
        float h1 = PerlinNoise((xz + offset * 75.f) / freq);
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }

    return floor(129+h*200);
}

glm::vec2 NoiseGenerator::generatePartition(glm::vec2 uv)
{
    glm::vec2 xz = uv;
    float terrainType = 0.5*(PerlinNoise(xz/256.f+vec2(100,200))+1);
    //terrainType = pow(terrainType,3);
    terrainType = smoothstep(0.3,0.7,terrainType);
    if(terrainType<0.5){
        return glm::vec2(1,0);
    }
    if(terrainType<0.7){
        return glm::mix(glm::vec2(0,1),glm::vec2(1,0),(0.7-terrainType)/0.2);
    }
    return glm::vec2(0,1);
}
