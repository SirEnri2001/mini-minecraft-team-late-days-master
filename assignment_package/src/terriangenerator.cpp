#include"terriangenerator.h"
#include<cmath>


//void TerrianGenerator::setContext(OpenGLContext* context){
//    this->context = context;
//}

void ChunkGenerator::setContext(OpenGLContext* context){
    this->context = context;
}

void ChunkGenerator::generateChunk(Chunk *chunk)
{
    for(int i = chunk->getMinX();i<16+chunk->getMinX();i++){
        for(int j = chunk->getMinZ();j<16+chunk->getMinZ();j++){
            chunk->setBlockAt((i%16+16)%16,0,(j%16+16)%16,BEDROCK);
            glm::vec2 uv(i,j);
            glm::vec2 partition = noiseGenerator.generatePartition(uv);
            BlockType blockType = partition[0]>0.99?DIRT:STONE;
            int h = partition[0]*noiseGenerator.generatePlain(uv)+partition[1]*noiseGenerator.generateMountain(uv);
            // TODO: UNCOMMENT THESE LINES TO GENERATE CAVE
            for(int k = 1;k<25;k++){
                if(noiseGenerator.generateCave(glm::vec3(i,k,j))>0.5){
                    chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,LAVA);
                }else{
                    chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,STONE);
                }
            }
            for(int k = 25;k<h;k++){
                if(noiseGenerator.generateCave(glm::vec3(i,k,j))>0.5){
                    chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,EMPTY);
                }else{
                    chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,blockType);
                }
            }
            for(int k = 128;k<h-1;k++){
                chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,blockType);
            }
            if(blockType==DIRT){
                chunk->setBlockAt((i%16+16)%16,h-1,(j%16+16)%16,GRASS);
            }else if(h>160){
                chunk->setBlockAt((i%16+16)%16,h-1,(j%16+16)%16,SNOW);
            }else{
                chunk->setBlockAt((i%16+16)%16,h-1,(j%16+16)%16,STONE);
            }
            if(h<140){
                for(int k = h-1;k<125;k++){
                    chunk->setBlockAt((i%16+16)%16,k,(j%16+16)%16,WATER);
                }
            }
        }
    }
}


MultithreadTerrainGenerator::MultithreadTerrainGenerator() :
    m_chunksHaveBlockTypeData_mutex{}, m_chunksHaveBlockTypeData{},
    m_chunksHaveVBOData_mutex{}, m_chunksHaveVBOData{}
{};

void MultithreadTerrainGenerator::checkAndGenerate(const std::vector<Chunk *> &newChunks)
{
    // Child Thread: queue empty chunks

    for (Chunk *c : newChunks) {
        std::thread t(generateChunkBlockTypeData, c, &m_chunksHaveBlockTypeData, &m_chunksHaveBlockTypeData_mutex);
#ifdef MULTI_THREADING
        t.detach();
#else
        t.join();
#endif
    }

    // Child Thread: generate VBO data
    {
        std::unique_lock<std::mutex> lock(m_chunksHaveBlockTypeData_mutex);
        for (Chunk *c : m_chunksHaveBlockTypeData) {
            c->generateIdx();
            c->generateAggregate();
            c->generateIdxfortransparent();
            c->generateAggregatefortransparent();
            std::thread t(generateVBOData, c, &m_chunksHaveVBOData, &m_chunksHaveVBOData_mutex);
#ifdef MULTI_THREADING
            t.detach();
#else
            t.join();
#endif
        }
        m_chunksHaveBlockTypeData.clear();
    }

    // Main Thread: transfer VBO data to GPU
    {
        std::unique_lock<std::mutex> lock(m_chunksHaveVBOData_mutex);
        // VBOData.insert(VBOData.end(), std::make_move_iterator(m_chunksHaveVBOData.begin()), std::make_move_iterator(m_chunksHaveVBOData.end()));
        for (const auto & data : m_chunksHaveVBOData) {
            data->mp_chunk->bufferVBOData(data->m_vertsOpaque, data->m_indicesOpaque);
            data->mp_chunk->bufferVBODatafortransparent(data->m_vertsTransparent, data->m_indicesTransparent);
        }
        m_chunksHaveVBOData.clear();
    }
}

void MultithreadTerrainGenerator::generateChunkBlockTypeData(Chunk *c, std::vector<Chunk *> *filledChunks, std::mutex *m)
{
    ChunkGenerator chunkGenerator;
    chunkGenerator.generateChunk(c);
    {
        std::unique_lock<std::mutex> lock(*m);
        filledChunks->push_back(c);
    }
}

void MultithreadTerrainGenerator::generateVBOData(Chunk *c, std::vector<std::unique_ptr<ChunkVBOData> > *chunksHaveVBOData, std::mutex *m)
{
    // No opengl context here!
    uPtr<ChunkVBOData> cVBOData = mkU<ChunkVBOData>(c);
    c->createVBOdata(cVBOData.get());
    {
        std::unique_lock<std::mutex> lock(*m);
        chunksHaveVBOData->push_back(std::move(cVBOData));
    }
}


