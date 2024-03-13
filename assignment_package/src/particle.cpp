#include "particle.h"
#include"noisegenerator.h"

Particle::Particle(OpenGLContext* context): Drawable(context)
{
    particles = std::make_unique<std::array<glm::vec4, 200000>>();

    NoiseGenerator gen;
    srand(114514);
    int ind = 0;
    for(auto& i : *particles){
        i=glm::vec4(rand()%1000/1250.0,0,0,1);
    }
}

void Particle::update(){

}

void Particle::createVBOdata(){
    m_count = 20000;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    GLCall(mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx));
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos));
    GLCall(mp_context->glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(glm::vec4), particles->data(), GL_STATIC_DRAW));

}

GLenum Particle::drawMode(){
    return GL_POINTS;
}
