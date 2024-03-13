#include "shaderprogram.h"
#include <QFile>
#include <QStringBuilder>
#include <QTextStream>
#include <QDebug>
#include <stdexcept>


ShaderProgram::ShaderProgram(OpenGLContext *context,bool hasGeomShader)
    : vertShader(), fragShader(), prog(),
      attrPos(-1), attrNor(-1), attrCol(-1),
    unifModel(-1), unifModelInvTr(-1), unifViewProj(-1), unifColor(-1),uniftexture(-1),uniftime(-1),unifcamera(-1),
    context(context), hasGeomShader(hasGeomShader)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile, const char* geomfile)
{
    if(hasGeomShader){
        // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
        GLCall(vertShader = context->glCreateShader(GL_VERTEX_SHADER));
        GLCall(fragShader = context->glCreateShader(GL_FRAGMENT_SHADER));
        GLCall(geomShader = context->glCreateShader(GL_GEOMETRY_SHADER));
        GLCall(prog = context->glCreateProgram());
        // Get the body of text stored in our two .glsl files
        QString qVertSource = qTextFileRead(vertfile);
        QString qFragSource = qTextFileRead(fragfile);
        QString qGeomSource = qTextFileRead(geomfile);

        char* vertSource = new char[qVertSource.size()+1];
        strcpy(vertSource, qVertSource.toStdString().c_str());
        char* fragSource = new char[qFragSource.size()+1];
        strcpy(fragSource, qFragSource.toStdString().c_str());
        char* geomSource = new char[qGeomSource.size()+1];
        strcpy(geomSource, qGeomSource.toStdString().c_str());


        // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
        GLCall(context->glShaderSource(vertShader, 1, (const char**)&vertSource, 0));
        GLCall(context->glShaderSource(geomShader, 1, (const char**)&geomSource, 0));
        GLCall(context->glShaderSource(fragShader, 1, (const char**)&fragSource, 0));
        // Tell OpenGL to compile the shader text stored above
        GLCall(context->glCompileShader(vertShader));
        GLCall(context->glCompileShader(geomShader));
        GLCall(context->glCompileShader(fragShader));
        // Check if everything compiled OK
        GLint compiled;
        GLCall(context->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            printShaderInfoLog(vertShader);
        }
        GLCall(context->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            printShaderInfoLog(fragShader);
        }
        GLCall(context->glGetShaderiv(geomShader, GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            printShaderInfoLog(geomShader);
        }

        // Tell prog that it manages these particular vertex and fragment shaders
        GLCall(context->glAttachShader(prog, vertShader));
        GLCall(context->glAttachShader(prog, geomShader));
        GLCall(context->glAttachShader(prog, fragShader));
        GLCall(context->glLinkProgram(prog));

        // Check for linking success
        GLint linked;
        GLCall(context->glGetProgramiv(prog, GL_LINK_STATUS, &linked));
        if (!linked) {
            printLinkInfoLog(prog);
        }
    }
    else{
        // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
        GLCall(vertShader = context->glCreateShader(GL_VERTEX_SHADER));
        GLCall(fragShader = context->glCreateShader(GL_FRAGMENT_SHADER));
        GLCall(prog = context->glCreateProgram());
        // Get the body of text stored in our two .glsl files
        QString qVertSource = qTextFileRead(vertfile);
        QString qFragSource = qTextFileRead(fragfile);

        char* vertSource = new char[qVertSource.size()+1];
        strcpy(vertSource, qVertSource.toStdString().c_str());
        char* fragSource = new char[qFragSource.size()+1];
        strcpy(fragSource, qFragSource.toStdString().c_str());


        // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
        GLCall(context->glShaderSource(vertShader, 1, (const char**)&vertSource, 0));
        GLCall(context->glShaderSource(fragShader, 1, (const char**)&fragSource, 0));
        // Tell OpenGL to compile the shader text stored above
        GLCall(context->glCompileShader(vertShader));
        GLCall(context->glCompileShader(fragShader));
        // Check if everything compiled OK
        GLint compiled;
        GLCall(context->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            printShaderInfoLog(vertShader);
        }
        GLCall(context->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            printShaderInfoLog(fragShader);
        }

        // Tell prog that it manages these particular vertex and fragment shaders
        GLCall(context->glAttachShader(prog, vertShader));
        GLCall(context->glAttachShader(prog, fragShader));
        GLCall(context->glLinkProgram(prog));

        // Check for linking success
        GLint linked;
        GLCall(context->glGetProgramiv(prog, GL_LINK_STATUS, &linked));
        if (!linked) {
            printLinkInfoLog(prog);
        }
    }
    // Get the handles to the variables stored in our shaders
    // See shaderprogram.h for more information about these variables

    GLCall(attrPos = context->glGetAttribLocation(prog, "vs_Pos"));
    GLCall(attrNor = context->glGetAttribLocation(prog, "vs_Nor"));
    GLCall(attrCol = context->glGetAttribLocation(prog, "vs_Col"));
    GLCall(attrUV = context->glGetAttribLocation(prog, "vs_UV"));
    if(attrCol == -1) GLCall(attrCol = context->glGetAttribLocation(prog, "vs_ColInstanced"));
    GLCall(attrPosOffset = context->glGetAttribLocation(prog, "vs_OffsetInstanced"));

    GLCall(unifModel            = context->glGetUniformLocation(prog, "u_Model"));
    GLCall(unifModelInvTr       = context->glGetUniformLocation(prog, "u_ModelInvTr"));
    GLCall(unifViewProj         = context->glGetUniformLocation(prog, "u_ViewProj"));
    GLCall(unifColor            = context->glGetUniformLocation(prog, "u_Color"));
    GLCall(uniftexture          = context->glGetUniformLocation(prog, "u_Texture"));
    GLCall(uniftime             = context->glGetUniformLocation(prog, "u_Time"));
    GLCall(unifcamera           = context->glGetUniformLocation(prog, "cameraposition"));
    GLCall(unifEye              = context->glGetUniformLocation(prog,"u_Eye"));
    GLCall(unifShadowViewProj   = context->glGetUniformLocation(prog, "u_lightViewProj"));
    GLCall(unifLightDirection   = context->glGetUniformLocation(prog, "u_lightDir"));
    GLCall(unifWeather=context->glGetUniformLocation(prog, "u_Weather"));
    GLCall(unifLookAt=context->glGetUniformLocation(prog, "u_LookAt"));
}

void ShaderProgram::useMe()
{
    GLCall(context->glUseProgram(prog));
}

void ShaderProgram::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        GLCall(context->glUniformMatrix4fv(unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                                           &model[0][0]));
    }

    if (unifModelInvTr != -1) {
        glm::mat4 modelinvtr = glm::inverse(glm::transpose(model));
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        GLCall(context->glUniformMatrix4fv(unifModelInvTr,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                                           &modelinvtr[0][0]));
    }
}

void ShaderProgram::setViewProjMatrix(const glm::mat4 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifViewProj != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    GLCall(context->glUniformMatrix4fv(unifViewProj,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                                           &vp[0][0]));
    }
}

void ShaderProgram::setGeometryColor(glm::vec4 color)
{
    useMe();

    if(unifColor != -1)
    {
    GLCall(context->glUniform4fv(unifColor, 1, &color[0]));
    }
}

void ShaderProgram::setLightViewProj(const glm::mat4 &vp)
{
    useMe();
    if(unifShadowViewProj == -1) return;

    GLCall(context->glUniformMatrix4fv(unifShadowViewProj,
                                       // How many matrices to pass
                                       1,
                                       // Transpose the matrix? OpenGL uses column-major, so no.
                                       GL_FALSE,
                                       // Pointer to the first element of the matrix
                                       &vp[0][0]));

}
void ShaderProgram::setEye(const glm::vec3 &eye){
    useMe();

    if(unifEye != -1)
    {
        GLCall(context->glUniform3fv(unifEye, 1, &eye[0]));
    }
}
void ShaderProgram::settexture(int slot){
    useMe();

    if(uniftexture != -1)
    {
        GLCall(context->glUniform1i(uniftexture,slot));
    }
}
void ShaderProgram::settime(int t) {
    useMe();

    if (uniftime != -1) {
        GLCall(context->glUniform1i(uniftime, t));
    }
}
void ShaderProgram::setcameraposition(const glm::vec3 &pos){
    useMe();
    if(unifcamera!=-1){
        GLCall(context->glUniform3fv(unifcamera, 1, &pos[0]));
    }
}

void ShaderProgram::setDimension(const glm::ivec2 &pos){
    useMe();
    if(unifDimension!=-1){
        GLCall(context->glUniform2iv(unifDimension, 1, &pos[0]));
    }
}

void ShaderProgram::setLightDirection(const glm::vec4 &dir)
{
    useMe();
    if(unifLightDirection!=-1){
        GLCall(context->glUniform4fv(unifLightDirection, 1, &dir[0]));
    }
}
//This function, as its name implies, uses the passed in GL widget
void ShaderProgram::draw(Drawable &d)
{
    useMe();
    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

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

    if (attrNor != -1 && d.bindNor()) {
        GLCall(context->glEnableVertexAttribArray(attrNor));
        GLCall(context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL));
    }

    if (attrCol != -1 && d.bindCol()) {
        GLCall(context->glEnableVertexAttribArray(attrCol));
        GLCall(context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, NULL));
    }

    if(attrUV!=-1&&d.bindUV()){
        GLCall(context->glEnableVertexAttribArray(attrUV));
        GLCall(context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL));
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    if(hasGeomShader){
        GLCall(context->glDrawArrays(GL_POINTS, 0, d.elemCount()));
    }else{
        GLCall(context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0));
    }

    if (attrPos != -1) GLCall(context->glDisableVertexAttribArray(attrPos));
    if (attrNor != -1) GLCall(context->glDisableVertexAttribArray(attrNor));
    if (attrCol != -1) GLCall(context->glDisableVertexAttribArray(attrCol));

    context->printGLErrorLog();
}

void ShaderProgram::drawInterleaved(Drawable &d)
{
    useMe();

    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO

    if (attrPos != -1 && d.bindAggregate()) {
        GLCall(context->glEnableVertexAttribArray(attrPos));
        GLCall(context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                              (void*) 0));
    }
    if (attrNor != -1 && d.bindAggregate()) {
        GLCall(context->glEnableVertexAttribArray(attrNor));
        GLCall(context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                       (void*) (sizeof(glm::vec4))));
    }
    if (attrCol != -1 && d.bindAggregate()) {
        GLCall(context->glEnableVertexAttribArray(attrCol));
        GLCall(context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                       (void*) (2 * sizeof(glm::vec4))));
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    GLCall(context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0));

    //context->printGLErrorLog();

    if (attrPos != -1) GLCall(context->glDisableVertexAttribArray(attrPos));
    if (attrNor != -1) GLCall(context->glDisableVertexAttribArray(attrNor));
    if (attrCol != -1) GLCall(context->glDisableVertexAttribArray(attrCol));
}
void ShaderProgram::drawInterleavedfortransparent(Drawable &d){
    useMe();

    if(d.elemCountfortransparent() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCountfortransparent()) + "!");
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO

    if (attrPos != -1 && d.bindAggregatefortransparent()) {
        GLCall(context->glEnableVertexAttribArray(attrPos));
        GLCall(context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                              (void*) 0));
    }
    if (attrNor != -1 && d.bindAggregatefortransparent()) {
        GLCall(context->glEnableVertexAttribArray(attrNor));
        GLCall(context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                                     (void*) (sizeof(glm::vec4))));
    }
    if (attrCol != -1 && d.bindAggregatefortransparent()) {
        GLCall(context->glEnableVertexAttribArray(attrCol));
        GLCall(context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 3 * sizeof(glm::vec4),
                                              (void*) (2 * sizeof(glm::vec4))));
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdxfortransparent();
    GLCall(context->glDrawElements(d.drawMode(), d.elemCountfortransparent(), GL_UNSIGNED_INT, 0));

    //context->printGLErrorLog();

    if (attrPos != -1) GLCall(context->glDisableVertexAttribArray(attrPos));
    if (attrNor != -1) GLCall(context->glDisableVertexAttribArray(attrNor));
    if (attrCol != -1) GLCall(context->glDisableVertexAttribArray(attrCol));
}

void ShaderProgram::drawInstanced(InstancedDrawable &d)
{
    useMe();

    if(d.elemCount() < 0) {
        throw std::out_of_range("Attempting to draw a drawable with m_count of " + std::to_string(d.elemCount()) + "!");
    }

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
        GLCall(context->glVertexAttribDivisor(attrPos, 0));
    }

    if (attrNor != -1 && d.bindNor()) {
        GLCall(context->glEnableVertexAttribArray(attrNor));
        GLCall(context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL));
        GLCall(context->glVertexAttribDivisor(attrNor, 0));
    }

    if (attrCol != -1 && d.bindCol()) {
        GLCall(context->glEnableVertexAttribArray(attrCol));
        GLCall(context->glVertexAttribPointer(attrCol, 3, GL_FLOAT, false, 0, NULL));
        GLCall(context->glVertexAttribDivisor(attrCol, 1));
    }

    if (attrPosOffset != -1 && d.bindOffsetBuf()) {
        GLCall(context->glEnableVertexAttribArray(attrPosOffset));
        GLCall(context->glVertexAttribPointer(attrPosOffset, 3, GL_FLOAT, false, 0, NULL));
        GLCall(context->glVertexAttribDivisor(attrPosOffset, 1));
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    GLCall(context->glDrawElementsInstanced(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0, d.instanceCount()));
    context->printGLErrorLog();

    if (attrPos != -1) GLCall(context->glDisableVertexAttribArray(attrPos));
    if (attrNor != -1) GLCall(context->glDisableVertexAttribArray(attrNor));
    if (attrCol != -1) GLCall(context->glDisableVertexAttribArray(attrCol));
    if (attrPosOffset != -1) GLCall(context->glDisableVertexAttribArray(attrPosOffset));

}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}

QString ShaderProgram::qTextFileRead(const char *fileName)
{
    QString text;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        text.append('\0');
    }
    return text;
}

void ShaderProgram::printShaderInfoLog(int shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    GLCall(context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen));

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        GLCall(context->glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog));
        qDebug() << "ShaderInfoLog:" << "\n" << infoLog << "\n";
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printLinkInfoLog(int prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    GLCall(context->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen));

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        GLCall(context->glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog));
        qDebug() << "LinkInfoLog:" << "\n" << infoLog << "\n";
        delete [] infoLog;
    }
}
