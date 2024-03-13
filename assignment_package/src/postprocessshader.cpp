#include "postprocessshader.h"
#include <QDateTime>

PostProcessShader::PostProcessShader(OpenGLContext *context)
    : ShaderProgram(context)
{}

PostProcessShader::~PostProcessShader()
{}

void PostProcessShader::draw(Drawable& d)
{
    useMe();

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO
    if (attrPos != -1 && d.bindPos()) {
        GLCall(context->glEnableVertexAttribArray(attrPos));
        GLCall(context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL));
        }
    if(d.bindUV()){
        GLCall(context->glEnableVertexAttribArray(attrUV));
        GLCall(context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL));
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    GLCall(context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0));

    if (attrPos != -1)
    {
        GLCall(context->glDisableVertexAttribArray(attrPos));
        GLCall(context->glDisableVertexAttribArray(attrUV));
    }
}

void PostProcessShader::setMode(Effect effect){
    useMe();
    GLCall(context->glUniform1i(unifEffect, effect));
}

void PostProcessShader::initAttributesUniforms(){
    useMe();
    GLCall(attrUV = context->glGetAttribLocation(prog, "vs_UV"));
    GLCall(unifSampler2D = context->glGetUniformLocation(prog, "u_RenderedTexture"));
    GLCall(unifSamplerDepth2D = context->glGetUniformLocation(prog, "u_DepthTexture"));
    GLCall(unifSamplerSky2D = context->glGetUniformLocation(prog, "u_SkyTexture"));
    GLCall(unifEffect = context->glGetUniformLocation(prog, "u_Effect"));
    GLCall(context->glUniform1i(unifEffect, NONE));
}
