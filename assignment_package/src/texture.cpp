#include "texture.h"
#include <QImage>
#include <QOpenGLWidget>

Texture::Texture(OpenGLContext *context)
    : context(context), m_textureHandle(-1), m_textureImage(nullptr)
{}

Texture::~Texture()
{}

void Texture::create(const char *texturePath)
{
    context->printGLErrorLog();

    QImage img(texturePath);
    img.convertToFormat(QImage::Format_ARGB32);
    img = img.mirrored();
    m_textureImage = std::make_shared<QImage>(img);
    GLCall(context->glGenTextures(1, &m_textureHandle));

    context->printGLErrorLog();
}

void Texture::load(int texSlot = 0)
{
    GLCall(context->glActiveTexture(GL_TEXTURE0 + texSlot));
    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_textureHandle));

    // These parameters need to be set for EVERY texture you create
    // They don't always have to be set to the values given here, but they do need
    // to be set
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                          m_textureImage->width(), m_textureImage->height(),
                                 0, GL_BGRA, GL_UNSIGNED_BYTE, m_textureImage->bits()));
    context->printGLErrorLog();
}


void Texture::bind(int texSlot = 0)
{
    GLCall(context->glActiveTexture(GL_TEXTURE0 + texSlot));
    GLCall(context->glBindTexture(GL_TEXTURE_2D, m_textureHandle));
}
