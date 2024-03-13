#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context, bool hasUV)
    : m_count(-1),m_countfortransparent(-1),m_bufIdx(),m_bufIdxfortransparent(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufAggregate(), m_bufaggregatefortransparent(),m_idxGenerated(false),m_idxGeneratedfortransparent(false),
    m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_aggregateGenerated(false),m_aggregatedGeneratedfortransparent(false),
    mp_context(context),hasUV(hasUV)
{}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    GLCall(mp_context->glDeleteBuffers(1, &m_bufIdx));
    GLCall(mp_context->glDeleteBuffers(1, &m_bufIdxfortransparent));
    GLCall(mp_context->glDeleteBuffers(1, &m_bufPos));
    GLCall(mp_context->glDeleteBuffers(1, &m_bufNor));
    GLCall(mp_context->glDeleteBuffers(1, &m_bufCol));
    m_idxGenerated = m_bufIdxfortransparent=m_posGenerated = m_norGenerated = m_colGenerated = false;
    m_count = -1;
    m_countfortransparent=-1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}
int Drawable::elemCountfortransparent()
{
    return m_countfortransparent;
}
void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    GLCall(mp_context->glGenBuffers(1, &m_bufIdx));
}
void Drawable::generateIdxfortransparent()
{
    m_idxGeneratedfortransparent = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    GLCall(mp_context->glGenBuffers(1, &m_bufIdxfortransparent));
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    GLCall(mp_context->glGenBuffers(1, &m_bufPos));
    GLCall(mp_context->glGenBuffers(1, &m_bufUV));
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    GLCall(mp_context->glGenBuffers(1, &m_bufNor));
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    GLCall(mp_context->glGenBuffers(1, &m_bufCol));
}

void Drawable::generateAggregate()
{
    m_aggregateGenerated = true;
    // Create a VBO on our GPU and store its handle in bufAggregate
    GLCall(mp_context->glGenBuffers(1, &m_bufAggregate));
}

void Drawable::generateAggregatefortransparent()
{
    m_aggregatedGeneratedfortransparent = true;
    // Create a VBO on our GPU and store its handle in bufAggregate
    GLCall(mp_context->glGenBuffers(1, &m_bufaggregatefortransparent));
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        GLCall(mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx));
    }
    return m_idxGenerated;
}
bool Drawable::bindIdxfortransparent(){
    if(m_idxGeneratedfortransparent) {
        GLCall(mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxfortransparent));
    }
    return m_idxGeneratedfortransparent;
}
bool Drawable::bindPos()
{
    if(m_posGenerated){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos));
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor));
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol));
    }
    return m_colGenerated;
}

bool Drawable::bindAggregate()
{
    if(m_aggregateGenerated){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufAggregate));
    }
    return m_aggregateGenerated;
}

bool Drawable::bindAggregatefortransparent()
{
    if(m_aggregatedGeneratedfortransparent){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufaggregatefortransparent));
    }
    return m_aggregatedGeneratedfortransparent;
}
bool Drawable::bindUV(){
    GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV));
    return hasUV;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    GLCall(mp_context->glGenBuffers(1, &m_bufPosOffset));
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset));
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        GLCall(mp_context->glDeleteBuffers(1, &m_bufPosOffset));
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        GLCall(mp_context->glDeleteBuffers(1, &m_bufCol));
        m_colGenerated = false;
    }
}
