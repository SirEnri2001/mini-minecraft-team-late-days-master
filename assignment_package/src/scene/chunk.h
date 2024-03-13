#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>
#include "drawable.h"

const static int CHUNK_SIZE = 16;

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW,LAVA,BEDROCK,
    WATER_XPOS,WATER_XNEG,WATER_ZPOS,WATER_ZNEG,WATER_XPOSZPOS,WATER_XPOSZNEG,WATER_XNEGZPOS,WATER_XNEGZNEG,WATERBLOCK,
    LAVA_XPOS,LAVA_XNEG,LAVA_ZPOS,LAVA_ZNEG,LAVA_XPOSZPOS,LAVA_XPOSZNEG,LAVA_XNEGZPOS,LAVA_XNEGZNEG,LAVABLOCK
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

//
enum VertexType : unsigned char
{
    POS, NOR, COL, UV,
};

const static std::unordered_set<BlockType> transparentBlocks {
    WATER,WATER_XPOS,WATER_XNEG,WATER_ZPOS,WATER_ZNEG,WATER_XPOSZPOS,WATER_XPOSZNEG,WATER_XNEGZPOS,WATER_XNEGZNEG,WATERBLOCK
};

const static std::unordered_set<BlockType> fluidBlocks{
    WATER_XPOS,WATER_XNEG,WATER_ZPOS,WATER_ZNEG,WATER_XPOSZPOS,WATER_XPOSZNEG,WATER_XNEGZPOS,WATER_XNEGZNEG,
        LAVA_XPOS,LAVA_XNEG,LAVA_ZPOS,LAVA_ZNEG,LAVA_XPOSZPOS,LAVA_XPOSZNEG,LAVA_XNEGZPOS,LAVA_XNEGZNEG
};
const static std::unordered_set<BlockType> fluidwaterBlocks{
    WATER_XPOS,WATER_XNEG,WATER_ZPOS,WATER_ZNEG,WATER_XPOSZPOS,WATER_XPOSZNEG,WATER_XNEGZPOS,WATER_XNEGZNEG
};
const static std::unordered_set<BlockType> fluidLAVABlocks{
LAVA_XPOS,LAVA_XNEG,LAVA_ZPOS,LAVA_ZNEG,LAVA_XPOSZPOS,LAVA_XPOSZNEG,LAVA_XNEGZPOS,LAVA_XNEGZNEG};


// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.
struct ChunkVBOData;
class Chunk : public Drawable{
public:
    const static std::unordered_map<Direction, glm::vec3, EnumHash> directionVec;
    const static std::vector<glm::vec3> directionChunkVec;
    const static std::vector<Direction> neighborDirections;
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    int minX, minZ;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    /* -------- helper functions -------- */
    // helper function to determine whether a face of a block
    // in this chunk should be drawn
    bool isFaceVisible(Direction dir, int x, int y, int z);
    // return the 4 vertex position of a block's face
    std::array<glm::vec3, 4> getBlockFaceVerts(Direction dir, int x, int y, int z);
public:
    Chunk(OpenGLContext *context, int x=0, int z=0);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    glm::vec4 getUVAttexture(BlockType block,Direction dir);
    std::array<glm::vec3, 4> getfluidyoffset(BlockType block,Direction dir);
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    std::vector<Chunk*> getNeighbors();
    inline int getMinX() const{
        return minX;
    }
    inline int getMinZ() const{
        return minZ;
    }

    void createVBOdata() override;
    void createVBOdata(ChunkVBOData *vboData);
    void bufferVBOData(std::vector<glm::vec4> interleavingData, std::vector<GLuint> indices);
    void bufferVBODatafortransparent(std::vector<glm::vec4> interleavingData, std::vector<GLuint> indices);
};

struct ChunkVBOData {
public:
    Chunk* mp_chunk;
    std::vector<glm::vec4> m_vertsOpaque, m_vertsTransparent;
    std::vector<GLuint> m_indicesOpaque, m_indicesTransparent;

public:
    ChunkVBOData(Chunk *c) : mp_chunk(c),
                             m_vertsOpaque{}, m_vertsTransparent{},
                             m_indicesOpaque{}, m_indicesTransparent{}
    {};

};



