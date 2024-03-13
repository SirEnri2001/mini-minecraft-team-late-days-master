#include "worldaxes.h"

WorldAxes::~WorldAxes()
{}

void WorldAxes::createVBOdata()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    glm::vec4 pos[6] = {glm::vec4(32,129,32,1), glm::vec4(40,129,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,137,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,129,40,1)};
    glm::vec4 col[6] = {glm::vec4(1,0,0,1), glm::vec4(1,0,0,1),
                        glm::vec4(0,1,0,1), glm::vec4(0,1,0,1),
                        glm::vec4(0,0,1,1), glm::vec4(0,0,1,1)};

    m_count = 6;

    generateIdx();
    GLCall(mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx));
    GLCall(mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW));
    generatePos();
    GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos));
    GLCall(mp_context->glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), pos, GL_STATIC_DRAW));
    generateCol();
    GLCall(mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol));
    GLCall(mp_context->glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), col, GL_STATIC_DRAW));
}

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
