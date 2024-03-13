#pragma once
#include "glm_includes.h"
class NoiseGenerator{
public:
    float WorleyNoise(glm::vec2 uv); // ranges [ 0, 1]
    float PerlinNoise(glm::vec2 uv); // ranges [-1, 1]
    float PerlinNoise3d(glm::vec3 uv);
    float FBM(glm::vec2 uv); // ranges [ 0, 1]
    float FBM3d(glm::vec3 uvw); // ranges [ 0, 1]
    float generateMountain(glm::vec2 uv);
    float generatePlain(glm::vec2 uv);
    float generateCave(glm::vec3 uvw);
    glm::vec2 generatePartition(glm::vec2 uv);
};
float smoothstep(float edge0, float edge1, float x);
float noise(glm::vec2 uv);
glm::vec2 smoothF(glm::vec2 uv);
glm::vec2 random2( glm::vec2 p );
