#include "framebuffer.h"

FrameBuffer::FrameBuffer(OpenGLContext* context) : context(context)
{
}


void FrameBuffer::create(){
    GLCall(context->glGenFramebuffers(1, &m_frameBufferObject));
    GLCall(context->glGenTextures(1, &m_frameTexture));
    GLCall(context->glGenTextures(1, &m_depthTexture));
    GLCall(context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));
    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_depthTexture));

    float shadowWidth = context->width() * context->devicePixelRatio();
    float shadowDepth = context->height() * context->devicePixelRatio();
    GLCall(context->glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        shadowWidth,
        shadowDepth,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0));


    GLCall(context->glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_frameTexture));
    GLCall(context->glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        context->width() * context->devicePixelRatio(),
        context->height() * context->devicePixelRatio(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(context->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0));
    GLCall(context->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_frameTexture, 0));
    if(context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        qDebug()<<"Initialize framebuffer failed ! Code:"<<context->glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(false);
    }
}


void FrameBuffer::useMe(){
    GLCall(context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));
}


void FrameBuffer::useDefault(){
    GLCall(context->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject()));
}

void FrameBuffer::bindTextureIntoSlot(unsigned int slotFrameTexture,unsigned int slotDepthTexture){
    GLCall(context->glActiveTexture(GL_TEXTURE0+slotDepthTexture));
    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_depthTexture));
    GLCall(context->glActiveTexture(GL_TEXTURE0+slotFrameTexture));
    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_frameTexture));
}
