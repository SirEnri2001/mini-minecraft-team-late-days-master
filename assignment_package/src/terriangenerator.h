#pragma once
#include "scene/chunk.h"
#include "noisegenerator.h"
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_set>
#include <set>


#define MULTI_THREADING

class ChunkGenerator{
    OpenGLContext* context;
    NoiseGenerator noiseGenerator;
public:
    void setContext(OpenGLContext* context);
    void generateChunk(Chunk* chunk);
};


class MultithreadTerrainGenerator {
public:
    std::mutex m_chunksHaveBlockTypeData_mutex;
    std::vector<Chunk*> m_chunksHaveBlockTypeData;

    std::mutex m_chunksHaveVBOData_mutex;
    std::vector<uPtr<ChunkVBOData>> m_chunksHaveVBOData;
    std::vector<std::thread> createdThreads;
public:
    MultithreadTerrainGenerator();

    void checkAndGenerate(const std::vector<Chunk*> &newChunks = {});


public:
    static void generateChunkBlockTypeData(Chunk* c, std::vector<Chunk*> *filledChunks, std::mutex *m);
    static void generateVBOData(Chunk* c, std::vector<uPtr<ChunkVBOData>> *chunksHaveVBOData, std::mutex *m);
};



