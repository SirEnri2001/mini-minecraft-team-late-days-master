#pragma once

#include "shaderprogram.h"

class PostProcessShader : public ShaderProgram
{
public:
    enum Effect{
        NONE, UNDERWATER, UNDERLAVA
    };
//    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrUV = -1; // A handle for the "in" vec2 representing the UV coordinates in the vertex shader
//    int unifTime;
    int unifSampler2D;
    int unifSamplerDepth2D;
    int unifEffect;
    int unifSamplerSky2D;
//    int unifDimensions; // A handle to the "uniform" ivec2 that stores the width and height of the texture being rendered
public:
    PostProcessShader(OpenGLContext* context);
    virtual ~PostProcessShader();

    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d) override;
    void setMode(Effect effect);
    void initAttributesUniforms();
};
