#include "chunk.h"

const std::unordered_map<Direction, glm::vec3, EnumHash> Chunk::directionVec {
    {XPOS, glm::vec3(1,0,0)},
    {XNEG, glm::vec3(-1,0,0)},
    {YPOS, glm::vec3(0,1,0)},
    {YNEG, glm::vec3(0,-1,0)},
    {ZPOS, glm::vec3(0,0,1)},
    {ZNEG, glm::vec3(0,0,-1)}
};

const std::vector<glm::vec3> Chunk::directionChunkVec {
    glm::vec3(1,0,0),
    glm::vec3(-1,0,0),
    glm::vec3(0,0,1),
    glm::vec3(0,0,-1),

};

const std::vector<Direction> Chunk::neighborDirections {
    XPOS, XNEG, ZPOS, ZNEG,
};


bool Chunk::isFaceVisible(Direction dir, int x, int y, int z)
{
    Chunk *blockChunk = this;
    BlockType thisBlock = getBlockAt(x, y, z);
    // skip empty blocks
    if (thisBlock == BlockType::EMPTY) {return false;}

    // get the neighbor block coordinate by direction
    glm::vec3 dirVec(x,y,z);
    dirVec += directionVec.at(dir);
    dirVec = glm::floor(dirVec);
    x = dirVec.x;
    y = dirVec.y;
    z = dirVec.z;

    // the case when the face is at maximum or minimum y bound, it will be rendered
    if (y >= 256 || y < 0) {
        return true;
    }

    // the case when the face is next to a neighbor chunk
    if (x >= 16 || x < 0 || z >= 16 || z < 0) {
        try{
            blockChunk = m_neighbors.at(dir);
            // it should draw if no neighbor is found
            if (!blockChunk) {
                if (transparentBlocks.find(thisBlock) == transparentBlocks.end()){
                    return true;
                }
                return false;
            }
        }
        catch (const std::out_of_range &e) {
            return true;
        }
        // recalculate local coordinate to get the correct neighbor block
        x = (x + 16) % 16;
        z = (z + 16) % 16;
    }

    // check whether the face is visible
    // (the face is visible if the connected neighbor block is transparent)
    BlockType neighborBlock = blockChunk->getBlockAt(x, y, z);

    // case of opaque face
    if (transparentBlocks.find(thisBlock) == transparentBlocks.end()) {
        // overlap with transparent neighbor face
        if ((transparentBlocks.find(neighborBlock) != transparentBlocks.end()) ||
                (neighborBlock == BlockType::EMPTY)) {
            return true;
        }
    } else {
        // case of water facing up and not next to another water block
        if (neighborBlock == BlockType::EMPTY) {
            return true;
        }
    }
    return false;

}

std::array<glm::vec3, 4> Chunk::getBlockFaceVerts(Direction dir , int x, int y, int z)
{
    std::array<glm::vec3, 4> posVerts;

    // the cube's coordinate origin, which is at one corner,
    // represented in the world's coordinate system
    glm::vec3 origin = glm::vec3(x, y, z) + glm::vec3(minX, 0, minZ);

    // assume using a target camera system start with facing -z direction;
    glm::vec3 posBottomLeft;
    glm::vec3 posBottomRight;
    glm::vec3 posTopLeft;
    glm::vec3 posTopRight;

    glm::vec3 upVec, rightVec;
    switch (dir) {
        case XPOS:
            posBottomLeft = origin + directionVec.at(XPOS) + directionVec.at(ZPOS);
            upVec = directionVec.at(YPOS);
            rightVec = directionVec.at(ZNEG);
            break;

        case XNEG:
            posBottomLeft = origin;
            upVec = directionVec.at(YPOS);
            rightVec = directionVec.at(ZPOS);
            break;

        case YPOS:
            posBottomLeft = origin + directionVec.at(YPOS) + directionVec.at(ZPOS);
            upVec = directionVec.at(ZNEG);
            rightVec = directionVec.at(XPOS);
            break;

        case YNEG:
            posBottomLeft = origin;
            upVec = directionVec.at(ZPOS);
            rightVec = directionVec.at(XPOS);
            break;

        case ZPOS:
            posBottomLeft = origin + directionVec.at(ZPOS);
            upVec = directionVec.at(YPOS);
            rightVec = directionVec.at(XPOS);

            break;

        case ZNEG:
            posBottomLeft = origin + directionVec.at(XPOS);
            upVec = directionVec.at(YPOS);
            rightVec = directionVec.at(XNEG);
            break;
    };

    posBottomRight = posBottomLeft + rightVec;
    posTopLeft = posBottomLeft + upVec;
    posTopRight = posTopLeft + rightVec;

    posVerts[0] = posBottomLeft;
    posVerts[1] = posBottomRight;
    posVerts[2] = posTopRight;
    posVerts[3] = posTopLeft;
    return posVerts;
}
glm::vec4 Chunk::getUVAttexture(BlockType block,Direction dir){
    glm::vec4 uvvalue;
    if(block==GRASS&&dir==YPOS){
            uvvalue=glm::vec4(8.f,13.f,0.f,0.f);
    }
    /*else if(block==LAVA_XPOS){
            uvvalue=glm::vec4(13.f,1.f,1.f,1.f);
    }*/
    else if((fluidwaterBlocks.find(block)!=fluidwaterBlocks.end())&&(dir==XPOS||dir==XNEG||dir==YNEG||dir==ZPOS||dir==ZNEG)){
          uvvalue=glm::vec4(13.f,3.f,1.f,0.f);
    }
    else if((fluidLAVABlocks.find(block)!=fluidLAVABlocks.end())&&(dir==XPOS||dir==XNEG||dir==YNEG||dir==ZPOS||dir==ZNEG)){
          uvvalue=glm::vec4(13.f,1.f,1.f,0.f);
    }
    else if(block==WATER_XPOS){
          uvvalue=glm::vec4(13.f,3.f,0.f,1.f);
    }
    else if(block==WATER_XNEG){
          uvvalue=glm::vec4(13.f,3.f,0.f,2.f);
    }
    else if(block==WATER_ZPOS){
          uvvalue=glm::vec4(13.f,3.f,0.f,3.f);
    }
    else if(block==WATER_ZNEG){
          uvvalue=glm::vec4(13.f,3.f,0.f,4.f);
    }
    else if(block==WATER_XPOSZPOS){
          uvvalue=glm::vec4(13.f,3.f,0.f,5.f);
    }
    else if(block==WATER_XPOSZNEG){
          uvvalue=glm::vec4(13.f,3.f,0.f,6.f);
    }
    else if(block==WATER_XNEGZPOS){
          uvvalue=glm::vec4(13.f,3.f,0.f,7.f);
    }
    else if(block==WATER_XNEGZNEG){
          uvvalue=glm::vec4(13.f,3.f,0.f,8.f);
    }
    else if(block==WATERBLOCK&&(dir==XPOS||dir==XNEG||dir==YNEG||dir==ZPOS||dir==ZNEG)){
          uvvalue=glm::vec4(13.f,3.f,1.f,0.f);
    }
    else if(block==LAVA_XPOS){
          uvvalue=glm::vec4(13.f,1.f,0.f,1.f);
    }
    else if(block==LAVA_XNEG){
          uvvalue=glm::vec4(13.f,1.f,0.f,2.f);
    }
    else if(block==LAVA_ZPOS){
          uvvalue=glm::vec4(13.f,1.f,0.f,3.f);
    }
    else if(block==LAVA_ZNEG){
          uvvalue=glm::vec4(13.f,1.f,0.f,4.f);
    }
    else if(block==LAVA_XPOSZPOS){
          uvvalue=glm::vec4(13.f,1.f,0.f,5.f);
    }
    else if(block==LAVA_XPOSZNEG){
          uvvalue=glm::vec4(13.f,1.f,0.f,6.f);
    }
    else if(block==LAVA_XNEGZPOS){
          uvvalue=glm::vec4(13.f,1.f,0.f,7.f);
    }
    else if(block==LAVA_XNEGZNEG){
          uvvalue=glm::vec4(13.f,1.f,0.f,8.f);
    }
    else if(block==LAVABLOCK&&(dir==XPOS||dir==XNEG||dir==YNEG||dir==ZPOS||dir==ZNEG)){
          uvvalue=glm::vec4(13.f,1.f,1.f,0.f);
    }

    else if(block==GRASS){
            uvvalue=glm::vec4(3.f,15.f,0.f,0.f);
    }
    else if(block==DIRT){
            uvvalue=glm::vec4(2.f,15.f,0.f,0.f);
    }
    else if(block==STONE){
            uvvalue=glm::vec4(1.f,15.f,0.f,0.f);
    }
    else if(block==WATER){
            uvvalue=glm::vec4(13.f,3.f,2.f,0.f);
    }
    else if(block==WATERBLOCK){
            uvvalue=glm::vec4(13.f,3.f,0.f,0.f);
    }
    else if(block==SNOW){
            uvvalue=glm::vec4(2.f,11.f,0.f,0.f);
    }
    else if(block==LAVA){
            uvvalue=glm::vec4(13.f,1.f,0.f,0.f);
    }
    else if(block==LAVABLOCK){
            uvvalue=glm::vec4(13.f,1.f,0.f,0.f);
    }

    else if(block==BEDROCK){
            uvvalue=glm::vec4(1.f,14.f,0.f,0.f);
    }
    uvvalue=uvvalue/16.f;
    return uvvalue;
}
std::array<glm::vec3, 4> Chunk::getfluidyoffset(BlockType block,Direction dir){
    std::array<glm::vec3, 4> yoffsets;
    glm::vec3 posBottomLeft{0};
    glm::vec3 posBottomRight{0};
    glm::vec3 posTopLeft{0};
    glm::vec3 posTopRight{0};
    if((block==WATER_XPOS||block==LAVA_XPOS)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,-0.3f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOS||block==LAVA_XPOS)&&(dir==XPOS)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
   /* else if((block==WATER_XPOS||block==LAVA_XPOS)&&(dir==ZNEG)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOS||block==LAVA_XPOS)&&(dir==ZPOS)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }*/

    else if((block==WATER_XNEG||block==LAVA_XNEG)&&(dir==YPOS)){
            posBottomLeft=glm::vec3(0.f,-0.3f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }
    else if((block==WATER_XNEG||block==LAVA_XNEG)&&(dir==XNEG)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    /*else if((block==WATER_XNEG||block==LAVA_XNEG)&&(dir==ZPOS)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEG||block==LAVA_XNEG)&&(dir==ZNEG)){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }*/

    else if((block==WATER_ZNEG||block==LAVA_ZNEG)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);

    }
    else if((block==WATER_ZNEG||block==LAVA_ZNEG)&&dir==ZNEG){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
   /* else if((block==WATER_ZNEG||block==LAVA_ZNEG)&&dir==XPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }
    else if((block==WATER_ZNEG||block==LAVA_ZNEG)&&dir==XNEG){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }*/

    else if((block==WATER_ZPOS||block==LAVA_ZPOS)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,-0.3f,0.f);
            posBottomRight=glm::vec3(0.f,-0.3f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }
    else if((block==WATER_ZPOS||block==LAVA_ZPOS)&&dir==ZPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOSZPOS||block==LAVA_XPOSZPOS)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,-0.3f,0.f);
            posBottomRight=glm::vec3(0.f,-0.3f,0.f);
            posTopLeft=glm::vec3(0.f,0.f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOSZPOS||block==LAVA_XPOSZPOS)&&dir==XPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOSZPOS||block==LAVA_XPOSZPOS)&&dir==ZPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEGZPOS||block==LAVA_XNEGZPOS)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,-0.3f,0.f);
            posBottomRight=glm::vec3(0.f,-0.3f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,0.f,0.f);
    }
    else if((block==WATER_XNEGZPOS||block==LAVA_XNEGZPOS)&&dir==XNEG){
            posBottomLeft=glm::vec3(0.f,-0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEGZPOS||block==LAVA_XNEGZPOS)&&dir==ZPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEGZNEG||block==LAVA_XNEGZNEG)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,-0.3f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEGZNEG||block==LAVA_XNEGZNEG)&&dir==XNEG){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XNEGZNEG||block==LAVA_XNEGZNEG)&&dir==ZNEG){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }

    else if((block==WATER_XPOSZNEG||block==LAVA_XPOSZNEG)&&dir==YPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,-0.3f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOSZNEG||block==LAVA_XPOSZNEG)&&dir==XPOS){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }
    else if((block==WATER_XPOSZNEG||block==LAVA_XPOSZNEG)&&dir==ZNEG){
            posBottomLeft=glm::vec3(0.f,0.f,0.f);
            posBottomRight=glm::vec3(0.f,0.f,0.f);
            posTopLeft=glm::vec3(0.f,-0.3f,0.f);
            posTopRight=glm::vec3(0.f,-0.3f,0.f);
    }

    yoffsets[0] = posBottomLeft;
    yoffsets[1] = posBottomRight;
    yoffsets[2] = posTopRight;
    yoffsets[3] = posTopLeft;
    return yoffsets;
}

void Chunk::bufferVBOData(std::vector<glm::vec4> interleavingData,
                            std::vector<GLuint> indices)
{
    m_count = indices.size();
    // bind index buffer
    if (!bindIdx()) {
        throw std::string("Error executing bufferVBOData: !m_idxGenerated");
    }
    GLCall(mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                                    indices.data(), GL_STATIC_DRAW));
    // bind vertex buffer
    if (!bindAggregate()) {
        throw std::string("Error executing bufferVBOData: !m_aggregateGenerated");
    }
    GLCall(mp_context->glBufferData(GL_ARRAY_BUFFER, interleavingData.size()*sizeof(glm::vec4),
                                    interleavingData.data(), GL_STATIC_DRAW));
}

void Chunk::bufferVBODatafortransparent(std::vector<glm::vec4> interleavingData,
                          std::vector<GLuint> indices)
{
    m_countfortransparent = indices.size();
    // bind index buffer
    if (!bindIdxfortransparent()) {
        throw std::string("Error executing bufferVBOData: !m_idxGeneratedfortransparent");
    }
    GLCall(mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                                    indices.data(), GL_STATIC_DRAW));
    // bind vertex buffer
    if (!bindAggregatefortransparent()) {
        throw std::string("Error executing bufferVBOData: !m_aggregateGeneratedfortransparent");
    }
    GLCall(mp_context->glBufferData(GL_ARRAY_BUFFER, interleavingData.size()*sizeof(glm::vec4),
                                    interleavingData.data(), GL_STATIC_DRAW));
}


Chunk::Chunk(OpenGLContext *context, int x, int z) :
    Drawable(context), m_blocks(), minX(x), minZ(z), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    unsigned long long idx = x + 16 * y + 16 * 256 * z;
    return m_blocks.at(idx);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

std::vector<Chunk *> Chunk::getNeighbors()
{
    std::vector<Chunk *> neighbors{};
    for (Direction d : neighborDirections) {
        auto n = m_neighbors.find(d);
        if (n == m_neighbors.end()) { continue; }
        if (!n->second) { continue; }
        neighbors.push_back(n->second);
    }
    return neighbors;
}

void Chunk::createVBOdata()
{
    if (!m_idxGenerated) { generateIdx(); }
    if (!m_aggregateGenerated) { generateAggregate(); }
    if (!m_aggregatedGeneratedfortransparent){generateAggregatefortransparent();}
    if(!m_idxGeneratedfortransparent){generateIdxfortransparent();}

    std::vector<glm::vec4> blockVertices;
    std::vector<GLuint> blockIndices;
    std::vector<glm::vec4> blockVerticesfortransparent;
    std::vector<GLuint> blockIndicesfortransparent;

    // vertex stride size = 3
    const int strideSize = std::vector<std::string>{"vs_Pos", "vs_Nor", "vs_UV"}.size();

    //offset for uv
    glm::vec4 uvoffsetforBR=glm::vec4(1.f/16.f,0.f,0.f,0.f);
    glm::vec4 uvoffsetforTR=glm::vec4(1.f/16.f,1.f/16.f,0.f,0.f);
    glm::vec4 uvoffsetforTL=glm::vec4(0.f,1.f/16.f,0.f,0.f);

    for (int x = 0; x < 16; x++) {
        // DEBUG
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                BlockType block = getBlockAt(x, y, z);
                // skip empty blocks
                if (block == BlockType::EMPTY) { continue; }

                // check face in each direction and buffer data
                // if block is opaque do following
                if(block==GRASS||block==DIRT||block==STONE||block==SNOW||block==LAVA||block==BEDROCK||block==LAVABLOCK||block==LAVA_XPOS||block==LAVA_XNEG||block==LAVA_ZPOS||block==LAVA_ZNEG||block==LAVA_XPOSZPOS||block==LAVA_XPOSZNEG||block==LAVA_XNEGZPOS||block==LAVA_XNEGZNEG){
                for (unsigned char i = XPOS; i <= ZNEG; i++) {
                    Direction dir = static_cast<Direction>(i);
                    if (!isFaceVisible(dir, x, y, z)) { continue; }
                    // get buffer data
                    std::array<glm::vec3, 4> verts = getBlockFaceVerts(dir, x, y, z);
                    std::array<glm::vec3, 4> yoffsets;
                    if(fluidBlocks.find(block)!=fluidBlocks.end()){
                        yoffsets=getfluidyoffset(block,dir);
                    }
                    glm::vec4 uvfortexture=getUVAttexture(block,dir);
                    // fill in vertex buffer
                    int startIdx = blockVertices.size() / strideSize;
                    for (const auto posVert : verts) {
                        // position vertex
                       // blockVertices.push_back(glm::vec4(posVert, 1));
                        // normal vertex
                        //blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                        // color vertex

                        //posbottomleft case
                        if(posVert==verts[0]){
                            // position vertex
                            blockVertices.push_back(glm::vec4(posVert+yoffsets[0], 1));
                            // normal vertex
                            blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                            // color vertex
                            blockVertices.push_back(uvfortexture);
                        }
                        //posbottomright case
                        else if(posVert==verts[1]){
                            // position vertex
                            blockVertices.push_back(glm::vec4(posVert+yoffsets[1], 1));
                            // normal vertex
                            blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                            blockVertices.push_back(uvfortexture+uvoffsetforBR);
                        }
                        //postopright case
                        else if(posVert==verts[2]){
                            // position vertex
                            blockVertices.push_back(glm::vec4(posVert+yoffsets[2], 1));
                            // normal vertex
                            blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                            blockVertices.push_back(uvfortexture+uvoffsetforTR);
                        }
                        //postopleft case
                        else{
                            // position vertex
                            blockVertices.push_back(glm::vec4(posVert+yoffsets[3], 1));
                            // normal vertex
                            blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                            blockVertices.push_back(uvfortexture+uvoffsetforTL);
                        }
                    }
                    // triangulate block face and fill in index buffer
                    for (const auto idx : {0,1,2,0,2,3}) {
                        blockIndices.push_back(idx + startIdx);
                    }
                }
              }
                //case when block is transparent
                else{
                for (unsigned char i = XPOS; i <= ZNEG; i++) {
                    Direction dir = static_cast<Direction>(i);
                    if (!isFaceVisible(dir, x, y, z)) { continue; }
                    // get buffer data
                    std::array<glm::vec3, 4> verts = getBlockFaceVerts(dir, x, y, z);

                    glm::vec4 uvfortexture=getUVAttexture(block,dir);
                    // fill in vertex buffer
                    int startIdx = blockVerticesfortransparent.size() / strideSize;
                    for (const auto posVert : verts) {
                        // position vertex
                        blockVerticesfortransparent.push_back(glm::vec4(posVert, 1));
                        // normal vertex
                        blockVerticesfortransparent.push_back(glm::vec4(directionVec.at(dir), 0));
                        // color vertex

                        //posbottomleft case
                        if(posVert==verts[0]){
                            blockVerticesfortransparent.push_back(uvfortexture);
                        }
                        //posbottomright case
                        else if(posVert==verts[1]){
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforBR);
                        }
                        //postopright case
                        else if(posVert==verts[2]){
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforTR);
                        }
                        //postopleft case
                        else{
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforTL);
                        }
                    }
                    // triangulate block face and fill in index buffer
                    for (const auto idx : {0,1,2,0,2,3}) {
                        blockIndicesfortransparent.push_back(idx + startIdx);
                    }
                }
                }
            }
        }
    }
    bufferVBOData(blockVertices, blockIndices);
    bufferVBODatafortransparent(blockVerticesfortransparent,blockIndicesfortransparent);
}

void Chunk::createVBOdata(ChunkVBOData *vboData)
{
    std::vector<glm::vec4> &blockVertices = vboData->m_vertsOpaque;
    std::vector<GLuint> &blockIndices = vboData->m_indicesOpaque;
    std::vector<glm::vec4> &blockVerticesfortransparent = vboData->m_vertsTransparent;
    std::vector<GLuint> &blockIndicesfortransparent = vboData->m_indicesTransparent;

    // vertex stride size = 3
    const int strideSize = std::vector<std::string>{"vs_Pos", "vs_Nor", "vs_UV"}.size();

    //offset for uv
    glm::vec4 uvoffsetforBR=glm::vec4(1.f/16.f,0.f,0.f,0.f);
    glm::vec4 uvoffsetforTR=glm::vec4(1.f/16.f,1.f/16.f,0.f,0.f);
    glm::vec4 uvoffsetforTL=glm::vec4(0.f,1.f/16.f,0.f,0.f);

    for (int x = 0; x < 16; x++) {
        // DEBUG
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                BlockType block = getBlockAt(x, y, z);
                // skip empty blocks
                if (block == BlockType::EMPTY) { continue; }

                // check face in each direction and buffer data
                // if block is opaque do following
                if(block==GRASS||block==DIRT||block==STONE||block==SNOW||block==LAVA||block==BEDROCK||block==LAVABLOCK||block==LAVA_XPOS||block==LAVA_XNEG||block==LAVA_ZPOS||block==LAVA_ZNEG||block==LAVA_XPOSZPOS||block==LAVA_XPOSZNEG||block==LAVA_XNEGZPOS||block==LAVA_XNEGZNEG){
                for (unsigned char i = XPOS; i <= ZNEG; i++) {
                    Direction dir = static_cast<Direction>(i);
                    if (!isFaceVisible(dir, x, y, z)) { continue; }
                    // get buffer data
                    std::array<glm::vec3, 4> verts = getBlockFaceVerts(dir, x, y, z);

                    glm::vec4 uvfortexture=getUVAttexture(block,dir);
                    // fill in vertex buffer
                    int startIdx = blockVertices.size() / strideSize;
                    for (const auto posVert : verts) {
                        // position vertex
                        blockVertices.push_back(glm::vec4(posVert, 1));
                        // normal vertex
                        blockVertices.push_back(glm::vec4(directionVec.at(dir), 0));
                        // color vertex

                        //posbottomleft case
                        if(posVert==verts[0]){
                            blockVertices.push_back(uvfortexture);
                        }
                        //posbottomright case
                        else if(posVert==verts[1]){
                            blockVertices.push_back(uvfortexture+uvoffsetforBR);
                        }
                        //postopright case
                        else if(posVert==verts[2]){
                            blockVertices.push_back(uvfortexture+uvoffsetforTR);
                        }
                        //postopleft case
                        else{
                            blockVertices.push_back(uvfortexture+uvoffsetforTL);
                        }
                    }
                    // triangulate block face and fill in index buffer
                    for (const auto idx : {0,1,2,0,2,3}) {
                        blockIndices.push_back(idx + startIdx);
                    }
                }
                }
                //case when block is transparent
                else{
                for (unsigned char i = XPOS; i <= ZNEG; i++) {
                    Direction dir = static_cast<Direction>(i);
                    if (!isFaceVisible(dir, x, y, z)) { continue; }
                    // get buffer data
                    std::array<glm::vec3, 4> verts = getBlockFaceVerts(dir, x, y, z);

                    glm::vec4 uvfortexture=getUVAttexture(block,dir);
                    // fill in vertex buffer
                    int startIdx = blockVerticesfortransparent.size() / strideSize;
                    for (const auto posVert : verts) {
                        // position vertex
                        blockVerticesfortransparent.push_back(glm::vec4(posVert, 1));
                        // normal vertex
                        blockVerticesfortransparent.push_back(glm::vec4(directionVec.at(dir), 0));
                        // color vertex

                        //posbottomleft case
                        if(posVert==verts[0]){
                            blockVerticesfortransparent.push_back(uvfortexture);
                        }
                        //posbottomright case
                        else if(posVert==verts[1]){
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforBR);
                        }
                        //postopright case
                        else if(posVert==verts[2]){
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforTR);
                        }
                        //postopleft case
                        else{
                            blockVerticesfortransparent.push_back(uvfortexture+uvoffsetforTL);
                        }
                    }
                    // triangulate block face and fill in index buffer
                    for (const auto idx : {0,1,2,0,2,3}) {
                        blockIndicesfortransparent.push_back(idx + startIdx);
                    }
                }
                }
            }
        }
    }
}
