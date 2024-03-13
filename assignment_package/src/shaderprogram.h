#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <openglcontext.h>
#include <glm_includes.h>
#include <glm/glm.hpp>

#include "drawable.h"


class ShaderProgram
{
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint geomShader;
    GLuint prog;       // A handle for the linked shader program stored in this class
    bool hasGeomShader = false;
    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
    int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader
    int attrUV;
    int attrPosOffset; // A handle for a vec3 used only in the instanced rendering shader

    int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
    int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
    int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
    int unifColor; // A handle for the "uniform" vec4 representing color of geometry in the vertex shader
    int uniftexture;
    int uniftime;
    int unifcamera;
    int unifEye;
    int unifDimension;
    int unifLightDirection;  // a handle for the "uniform" vec4 representing the light's direction

    int unifShadowViewProj; // A handle for the "uniform" mat4 that transform vertex to the sun's view
    int unifWeather;
    int unifLookAt;

public:
    ShaderProgram(OpenGLContext* context, bool hasGeomShader = false);
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile, const char * geomfile = nullptr);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat4 &model);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewProjMatrix(const glm::mat4 &vp);
    // Pass the given color to this shader on the GPU
    void setGeometryColor(glm::vec4 color);
    // Pass the given light source's Projection * View matrix to this shader on the GPU
    void setLightViewProj(const glm::mat4 &vp);

    void settexture(int slot);
    void settime(int t);
    void setcameraposition(const glm::vec3 &pos);
    void setEye(const glm::vec3 &eye);
    void setDimension(const glm::ivec2 &);
    void setLightDirection(const glm::vec4 &);
    // Draw the given object to our screen using this ShaderProgram's shaders
    virtual void draw(Drawable &d);
    // Draw the given object with interleaving VBO to our screen
    // using this ShaderProgram's shaders
    void drawInterleaved(Drawable &d);
    //draw given transparent object
    void drawInterleavedfortransparent(Drawable &d);
    // Draw the given object to our screen multiple times using instanced rendering
    void drawInstanced(InstancedDrawable &d);
    // Utility function used in create()
    char* textFileRead(const char*);
    // Utility function that prints any shader compilation errors to the console
    void printShaderInfoLog(int shader);
    // Utility function that prints any shader linking errors to the console
    void printLinkInfoLog(int prog);

    QString qTextFileRead(const char*);

protected:
    OpenGLContext* context;   // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                            // we need to pass our OpenGL context to the Drawable in order to call GL functions
                            // from within this class.
};


#endif // SHADERPROGRAM_H
