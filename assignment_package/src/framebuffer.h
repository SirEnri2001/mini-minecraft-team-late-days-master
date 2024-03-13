#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "openglcontext.h"

class FrameBuffer
{
private:
    OpenGLContext* context;
public:
    FrameBuffer(OpenGLContext* context);
    unsigned int m_frameBufferObject;
    unsigned int m_frameTexture;
    unsigned int m_depthTexture;

    void useMe();
    void useDefault();
    void create();
    void bindTextureIntoSlot(unsigned int slotFrameTexture,unsigned int slotDepthTexture);
};

#endif // FRAMEBUFFER_H
