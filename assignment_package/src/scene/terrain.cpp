#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include <cassert>
#include "terriangenerator.h"

#define ASSERT(X) if(!x){__debugbreak();}

int Terrain::clampToCoordinate(float x, int len)
{
    int r = len * static_cast<int>(glm::floor(x / static_cast<float>(len)));
    return r;
}

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context), m_terrainGenerator{}
{}

Terrain::~Terrain() {
    for (const auto &i :  m_chunks) {
        i.second->destroyVBOdata();
    }
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
//        throw std::out_of_range("getBlockAt: Coordinates " + std::to_string(x) +
//                                " " + std::to_string(y) + " " +
//                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    auto p = m_chunks.find(toKey(16 * xFloor, 16 * zFloor));
    return p != m_chunks.end() && p->second;
}

bool Terrain::hasChunkAt(float x, float z) const
{
    int xI = glm::floor(x);
    int zI = glm::floor(z);
    return hasChunkAt(xI, zI);
}

bool Terrain::hasTerrainAt(float x, float z) const
{
    int xI = clampToCoordinate(x, TERRAIN_SIZE);
    int zI = clampToCoordinate(z, TERRAIN_SIZE);

    auto p = m_generatedTerrain.find(toKey(xI, zI));
    return p != m_generatedTerrain.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::updateneighborchunk(int blocklocalX,int blocklocalz,int chunkoriginx,int chunkoriginz,std::unordered_set<Chunk *> &updatechunks){
    if (blocklocalX == 0) {
        updatechunks.insert(getChunkAt(chunkoriginx-1, chunkoriginz).get());
    }
    if (blocklocalX == 15) {
        updatechunks.insert(getChunkAt(chunkoriginx+16, chunkoriginz).get());
    }
    if (blocklocalz == 0) {
        updatechunks.insert(getChunkAt(chunkoriginx, chunkoriginz-1).get());
    }
    if (blocklocalz == 15) {
        updatechunks.insert(getChunkAt(chunkoriginx, chunkoriginz+16).get());
    }
}
void Terrain::fluidsimulation(int x, int y, int z,BlockType t,std::unordered_set<Chunk*> &updatechunks){
    uPtr<Chunk> &c = getChunkAt(x, z);
    int chunkOriginX = clampToCoordinate(x, CHUNK_SIZE);
    int chunkOriginZ = clampToCoordinate(z, CHUNK_SIZE);
    int blockLocalX = x - static_cast<int>(chunkOriginX);
    int blockLocalZ = z - static_cast<int>(chunkOriginZ);
    if(t==LAVABLOCK){
        c->setBlockAt(static_cast<unsigned int>(blockLocalX),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ),t);
        while(getBlockAt(x,y-1,z) == EMPTY){
            y-=1;
            c->setBlockAt(static_cast<unsigned int>(blockLocalX), static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ),t);

        }
        updatechunks.insert(c.get());
        updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        //LAVA_XPOS case
        if(getBlockAt(x+1,y,z) == EMPTY){
            int temp=x+1;
            uPtr<Chunk> &c = getChunkAt(temp, z);
            int chunkOriginX1 = clampToCoordinate(temp, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(z, CHUNK_SIZE);
            int blockLocalX1 = temp - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = z - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XPOS);
            int offset=-1;
            while(getBlockAt(temp,y+offset,z) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //LAVA_ZPOS case
        if(getBlockAt(x,y,z+1) == EMPTY){
            int temp=z+1;
            uPtr<Chunk> &c = getChunkAt(x, temp);
            int chunkOriginX1 = clampToCoordinate(x, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = x - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_ZPOS);
            int offset=-1;
            while(getBlockAt(x,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //LAVA_XNEG case
        if(getBlockAt(x-1,y,z) == EMPTY){
            int temp=x-1;
            uPtr<Chunk> &c = getChunkAt(temp, z);
            int chunkOriginX1 = clampToCoordinate(temp, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(z, CHUNK_SIZE);
            int blockLocalX1 = temp - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = z - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XNEG);
            int offset=-1;
            while(getBlockAt(temp,y+offset,z) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //LAVA_ZNEG case
        if(getBlockAt(x,y,z-1) == EMPTY){
            int temp=z-1;
            uPtr<Chunk> &c = getChunkAt(x, temp);
            int chunkOriginX1 = clampToCoordinate(x, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = x - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_ZNEG);
            int offset=-1;
            while(getBlockAt(x,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //LAVA_XZPOS case
        if(getBlockAt(x+1,y,z+1) == EMPTY){
            int temp=z+1;
            int temp2=x+1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XPOSZPOS);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //LAVA_XNEGZPOS case
        if(getBlockAt(x-1,y,z+1) == EMPTY){
            int temp=z+1;
            int temp2=x-1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XNEGZPOS);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //LAVA_XNEGZNEG case
        if(getBlockAt(x-1,y,z-1) == EMPTY){
            int temp=z-1;
            int temp2=x-1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XNEGZNEG);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //LAVA_XPOSZEG case
        if(getBlockAt(x+1,y,z-1) == EMPTY){
            int temp=z-1;
            int temp2=x+1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), LAVA_XPOSZNEG);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),LAVABLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
    }
    else{
        c->setBlockAt(static_cast<unsigned int>(blockLocalX),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ),t);
        while(getBlockAt(x,y-1,z) == EMPTY){
            y-=1;
            c->setBlockAt(static_cast<unsigned int>(blockLocalX), static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ),t);

        }
        updatechunks.insert(c.get());
        updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        //WATER_XPOS case
        if(getBlockAt(x+1,y,z) == EMPTY){
            int temp=x+1;
            uPtr<Chunk> &c = getChunkAt(temp, z);
            int chunkOriginX1 = clampToCoordinate(temp, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(z, CHUNK_SIZE);
            int blockLocalX1 = temp - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = z - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XPOS);
            int offset=-1;
            while(getBlockAt(temp,y+offset,z) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //WATER_ZPOS case
        if(getBlockAt(x,y,z+1) == EMPTY){
            int temp=z+1;
            uPtr<Chunk> &c = getChunkAt(x, temp);
            int chunkOriginX1 = clampToCoordinate(x, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = x - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_ZPOS);
            int offset=-1;
            while(getBlockAt(x,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //WATER_XNEG case
        if(getBlockAt(x-1,y,z) == EMPTY){
            int temp=x-1;
            uPtr<Chunk> &c = getChunkAt(temp, z);
            int chunkOriginX1 = clampToCoordinate(temp, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(z, CHUNK_SIZE);
            int blockLocalX1 = temp - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = z - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XNEG);
            int offset=-1;
            while(getBlockAt(temp,y+offset,z) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //WATER_ZNEG case
        if(getBlockAt(x,y,z-1) == EMPTY){
            int temp=z-1;
            uPtr<Chunk> &c = getChunkAt(x, temp);
            int chunkOriginX1 = clampToCoordinate(x, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = x - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_ZNEG);
            int offset=-1;
            while(getBlockAt(x,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //WATER_XZPOS case
        if(getBlockAt(x+1,y,z+1) == EMPTY){
            int temp=z+1;
            int temp2=x+1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XPOSZPOS);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }

        //WATER_XNEGZPOS case
        if(getBlockAt(x-1,y,z+1) == EMPTY){
            int temp=z+1;
            int temp2=x-1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XNEGZPOS);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //WATER_XNEGZNEG case
        if(getBlockAt(x-1,y,z-1) == EMPTY){
            int temp=z-1;
            int temp2=x-1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XNEGZNEG);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
        //WATER_XPOSZEG case
        if(getBlockAt(x+1,y,z-1) == EMPTY){
            int temp=z-1;
            int temp2=x+1;
            uPtr<Chunk> &c = getChunkAt(temp2, temp);
            int chunkOriginX1 = clampToCoordinate(temp2, CHUNK_SIZE);
            int chunkOriginZ1 = clampToCoordinate(temp, CHUNK_SIZE);
            int blockLocalX1 = temp2 - static_cast<int>(chunkOriginX1);
            int blockLocalZ1 = temp - static_cast<int>(chunkOriginZ1);
            c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y),static_cast<unsigned int>(blockLocalZ1), WATER_XPOSZNEG);
            int offset=-1;
            while(getBlockAt(temp2,y+offset,temp) == EMPTY){
                c->setBlockAt(static_cast<unsigned int>(blockLocalX1),static_cast<unsigned int>(y+offset),static_cast<unsigned int>(blockLocalZ1),WATERBLOCK);
                offset -= 1;
            }
            updatechunks.insert(c.get());
            updateneighborchunk(blockLocalX,blockLocalZ,chunkOriginX,chunkOriginZ,updatechunks);
        }
    }
}
void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {

        if(t==LAVABLOCK||t==WATERBLOCK){
             std::unordered_set<Chunk*> updateChunks;
            fluidsimulation(x,y,z,t,updateChunks);
             for (Chunk* chunk : updateChunks) {
                chunk->createVBOdata();
             }
        }
        else{
            uPtr<Chunk> &c = getChunkAt(x, z);
            int chunkOriginX = clampToCoordinate(x, CHUNK_SIZE);
            int chunkOriginZ = clampToCoordinate(z, CHUNK_SIZE);
            int blockLocalX = x - static_cast<int>(chunkOriginX);
            int blockLocalZ = z - static_cast<int>(chunkOriginZ);
        c->setBlockAt(static_cast<unsigned int>(blockLocalX),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(blockLocalZ),
                      t);
            // update the chunk's VBO data
            c->createVBOdata();

            // update the neighbor chunk's VBO data if needed


            if (blockLocalX == 0) {
                getChunkAt(chunkOriginX-1, chunkOriginZ)->createVBOdata();
            }
            if (blockLocalX == 15) {
                getChunkAt(chunkOriginX+16, chunkOriginZ)->createVBOdata();
            }
            if (blockLocalZ == 0) {
                getChunkAt(chunkOriginX, chunkOriginZ-1)->createVBOdata();
            }
            if (blockLocalZ == 15) {
                getChunkAt(chunkOriginX, chunkOriginZ+16)->createVBOdata();
            }
        }

    }
    else {
        throw std::out_of_range("setBlockAt: Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = std::move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(ShaderProgram *shaderProgram) {
    for(auto& idChunk : m_chunks){
        Chunk& chunk = *idChunk.second;
        shaderProgram->drawInterleaved(chunk);
    }

    for(auto& idChunk : m_chunks){
        Chunk& chunk = *idChunk.second;
        shaderProgram->drawInterleavedfortransparent(chunk);
    }

}

void Terrain::drawOpaque(ShaderProgram *shaderProgram)
{
    for(auto& idChunk : m_chunks){
        Chunk& chunk = *idChunk.second;
        shaderProgram->drawInterleaved(chunk);
    }
}

void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    Chunk* c;
    std::vector<Chunk*> initChunks;
    ChunkGenerator cg;
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            c = instantiateChunkAt(x, z);
            c->m_count = 0;
            c->m_countfortransparent = 0;
            initChunks.push_back(c);
            cg.generateChunk(c);
        }
    }
    for (Chunk *cp : initChunks) {
        cp->createVBOdata();
    }
}

#define MULTI
void Terrain::tryExpand(float playerX, float playerZ, float dT)
{

#ifndef MULTI
    m_terrainGenTimer += dT;
    if (m_terrainGenTimer < TERRAIN_GENERATE_CD) {return;}
    ChunkGenerator chunkGenerator;

    playerX = glm::floor(playerX);
    playerZ = glm::floor(playerZ);
    // check and generate terrains within zone
    int playerTerrainX = clampToCoordinate(playerX, TERRAIN_SIZE);
    int playerTerrainZ = clampToCoordinate(playerZ, TERRAIN_SIZE);
    for (int i = -GENERATION_ZONE_SIZE; i <= GENERATION_ZONE_SIZE; i++) {
        for (int j = -GENERATION_ZONE_SIZE; j <= GENERATION_ZONE_SIZE; j++) {
            int terrainX = playerTerrainX + i * TERRAIN_SIZE,
                terrainZ = playerTerrainZ + j * TERRAIN_SIZE;
            if (hasTerrainAt(terrainX, terrainZ)) { continue; }
            // generate chunks in the terrain
            m_generatedTerrain.insert(toKey(terrainX, terrainZ));
            std::vector<Chunk*> newChunks;
            const int chunksCount = TERRAIN_SIZE / CHUNK_SIZE;
            for (int ci = 0; ci < chunksCount; ci++) {
                for (int cj = 0; cj < chunksCount; cj++) {
                    int chunkX = terrainX + ci * CHUNK_SIZE,
                        chunkZ = terrainZ + cj * CHUNK_SIZE;
                    Chunk *c = instantiateChunkAt(chunkX, chunkZ);
                    c->m_count = 0;
                    c->m_countfortransparent = 0;
                    // create block data
                    chunkGenerator.generateChunk(c);
                    // create VBO data
                    ChunkVBOData data(c);
                    c->createVBOdata(&data);
                    data.mp_chunk->bufferVBOData(data.m_vertsOpaque, data.m_indicesOpaque);
                    data.mp_chunk->bufferVBODatafortransparent(data.m_vertsTransparent, data.m_indicesTransparent);

                }
            }
        }
    }
    m_terrainGenTimer = 0;
#else
    m_terrainGenTimer += dT;
    if (m_terrainGenTimer < TERRAIN_GENERATE_CD) {return;}
    ChunkGenerator chunkGenerator;

    playerX = glm::floor(playerX);
    playerZ = glm::floor(playerZ);
    // check and generate terrains within zone
    int playerTerrainX = clampToCoordinate(playerX, TERRAIN_SIZE);
    int playerTerrainZ = clampToCoordinate(playerZ, TERRAIN_SIZE);
    for (int i = -GENERATION_ZONE_SIZE; i <= GENERATION_ZONE_SIZE; i++) {
        for (int j = -GENERATION_ZONE_SIZE; j <= GENERATION_ZONE_SIZE; j++) {
            int terrainX = playerTerrainX + i * TERRAIN_SIZE,
                terrainZ = playerTerrainZ + j * TERRAIN_SIZE;
            if (hasTerrainAt(terrainX, terrainZ)) { continue; }
            // generate chunks in the terrain
            m_generatedTerrain.insert(toKey(terrainX, terrainZ));

            std::vector<Chunk*> newChunks{};
            const int chunksCount = TERRAIN_SIZE / CHUNK_SIZE;
            for (int ci = 0; ci < chunksCount; ci++) {
                for (int cj = 0; cj < chunksCount; cj++) {
                    int chunkX = terrainX + ci * CHUNK_SIZE,
                        chunkZ = terrainZ + cj * CHUNK_SIZE;
                    Chunk *c = instantiateChunkAt(chunkX, chunkZ);
                    c->m_count = 0;
                    c->m_countfortransparent = 0;
                    newChunks.push_back(c);
                }
            }
            m_terrainGenerator.checkAndGenerate(newChunks);
        }
    }
    m_terrainGenerator.checkAndGenerate();
    m_terrainGenTimer = 0;
#endif
}



















