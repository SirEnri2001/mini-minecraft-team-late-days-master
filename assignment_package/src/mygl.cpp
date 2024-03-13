#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>



MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomQuad(this),

    m_worldAxes(this),
    m_progLambert(this),
    m_progFlat(this),
    m_progShadowMap(this),
    m_progParticle(this, true),
    m_progPostprocessCurrent(this),
    m_progSky(this),
    currenttime(QDateTime::currentMSecsSinceEpoch()),
    m_playerFrameBuffer(this),
    m_shadowMapFrameBuffer(this),
    m_skyFrameBuffer(this),
    m_miniMapFrameBuffer(this),
    m_particles(this),
    m_terrain(this),
    m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain),
    m_texture(this),
    timeforuv(0)

{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    GLCall(glDeleteVertexArrays(1, &vao));
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{

    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_LEQUAL));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // Set the color with which the screen is filled at the start of each render call.
    GLCall(glClearColor(0.f, 0, 0.0f, 1));
    // Create a Vertex Attribute Object
    GLCall(glGenVertexArrays(1, &vao));
    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    m_geomQuad.createVBOdata();
    m_particles.createVBOdata();
    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the shadow texture shader
    m_progShadowMap.create(":/glsl/flat.vert.glsl", ":/glsl/shadowMap.frag.glsl");
    //create texture
    m_texture.create(":/textures/minecraft_textures_all.png");
    m_texture.load(0);
    m_progPostprocessCurrent.create(":/glsl/passthrough.vert.glsl", ":/glsl/underwater.post.frag.glsl");
    m_progPostprocessCurrent.initAttributesUniforms();
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");
    m_progParticle.create(":/glsl/particle.vert.glsl",":/glsl/particle.frag.glsl", ":/glsl/particle.geom.glsl");
    m_playerFrameBuffer.create();
    m_shadowMapFrameBuffer.create();
    m_skyFrameBuffer.create();
    m_miniMapFrameBuffer.create();
    //m_progPostprocessCurrent.setupMemberVars();

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    GLCall(glBindVertexArray(vao));

    //setinitial value for lastmousex and lastmousey
    m_inputs.lastmouseX=width()/2.f;
    m_inputs.lastmouseY=height()/2.f;

    m_terrain.CreateTestScene();
    weather = Weather::CLEAR;
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(viewproj);
    m_progPostprocessCurrent.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    //added so Postprocessshader will not have any visual effects after player leave the water or lava
    m_progPostprocessCurrent.setMode(PostProcessShader::NONE);
    BlockType eyeBlockType = m_terrain.getBlockAt(glm::vec3(m_player.mcr_position.x,m_player.mcr_position.y+1.5f,m_player.mcr_position.z));
    if(eyeBlockType==WATER){
        m_progPostprocessCurrent.setMode(PostProcessShader::UNDERWATER);
    }
    if(eyeBlockType==EMPTY){
        m_progPostprocessCurrent.setMode(PostProcessShader::NONE);
    }
    if(eyeBlockType==LAVA){
        m_progPostprocessCurrent.setMode(PostProcessShader::UNDERLAVA);
    }
    float dT = (QDateTime::currentMSecsSinceEpoch()-currenttime)/500.f;
    m_player.tick(dT, m_inputs);
    currenttime=QDateTime::currentMSecsSinceEpoch();
    timeforuv++;
    m_terrain.tryExpand(m_player.mcr_position.x, m_player.mcr_position.z, dT);
    m_progLambert.settime(timeforuv);
    m_progLambert.setcameraposition(m_player.mcr_camera.mcr_position);
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setModelMatrix(glm::mat4());
    m_progSky.setEye(m_player.mcr_camera.mcr_position);
    m_progSky.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    m_progParticle.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progParticle.setModelMatrix(glm::mat4(1));
    m_progParticle.setEye(m_player.mcr_camera.mcr_position);
    m_progParticle.settime(timeforuv);
    m_progSky.useMe();
    GLCall(glUniform1i(m_progSky.unifWeather,weather));
    m_progParticle.useMe();
    GLCall(glUniform1i(m_progParticle.unifWeather, weather));
    GLCall(glUniformMatrix4fv(m_progParticle.unifLookAt,1,GL_FALSE, &(glm::inverse(m_player.mcr_camera.getLookAt())[0][0])));
    glm::mat4 identity(1.0);
    //GLCall(glUniformMatrix4fv(m_progParticle.unifLookAt,1,GL_FALSE, &(identity[0][0])));
    //m_progSky.setDimension(glm::ivec2(this->width(),this->height()));
    m_progSky.settime(timeforuv);
    m_particles.update();
    m_progPostprocessCurrent.settime(timeforuv);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
#define SHADOWTEST
void MyGL::paintGL() {

////
    glm::mat3 sunRot =glm::mat3(
        1,0,0,
        0,cos(timeforuv/100.0),-sin(timeforuv/100.0),
        0,sin(timeforuv/100.0),cos(timeforuv/100.0)
        );
    glm::vec3 sunDir = sunRot*glm::normalize(glm::vec3(0.0,0.0,-1.0));
///
    m_skyFrameBuffer.useMe();
    GLCall(glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio()));
    GLCall(glClearColor(.5f, .5f, 1.0f, 0.f));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_BACK));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    m_texture.bind(0);
    m_progLambert.settexture(0);
    m_progSky.draw(m_geomQuad);
    // shadow-map frame buffer setup
    m_shadowMapFrameBuffer.useMe();
    int shadowWidth = this->width() * devicePixelRatio();
    int shadowHeight = this->height() * devicePixelRatio();
    GLCall(glViewport(0,0,shadowWidth, shadowHeight));
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));
    GLCall(glDrawBuffer(GL_NONE));
    GLCall(glClearColor(0.5f, .5f, 1.0f, 0));
    // calculate shadow texture
    m_progShadowMap.useMe();
    float shadowRange = 128;
    glm::vec3 lightDir = glm::normalize(sunDir);
    glm::vec3 lightTarget = m_player.mcr_camera.mcr_position;
        lightTarget.x = Terrain::clampToCoordinate(lightTarget.x, TERRAIN_SIZE) + 0.5 * TERRAIN_SIZE;
        lightTarget.y = 0;
        lightTarget.z = Terrain::clampToCoordinate(lightTarget.z, TERRAIN_SIZE) + 0.5 * TERRAIN_SIZE;
    glm::vec3 lightPos = lightTarget + lightDir;
        glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0,1,1));
    glm::mat4 lightProj = glm::ortho(-shadowRange, shadowRange, -shadowRange, shadowRange, -256.f, 32.f);
    glm::mat4 lightViewProj = lightProj * lightView;
    m_progShadowMap.setModelMatrix(glm::mat4());
    m_progShadowMap.setViewProjMatrix(lightViewProj);
    // disable backface culling to avoid peter-panning
    GLCall(glDisable(GL_CULL_FACE));
    m_terrain.drawOpaque(&m_progShadowMap);

    // normal draw frame buffer setup
    m_playerFrameBuffer.useMe();
    GLCall(glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio()));
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));
    GLCall(glClearColor(0.5f, 0.5f, 1.0f, 0));
    GLCall(glDrawBuffer(GL_COLOR_ATTACHMENT0));
    // lambert shader
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setModelMatrix(glm::mat4());
    m_progLambert.setLightViewProj(lightViewProj);
    glm::vec4 light = glm::vec4(lightDir, 0);
    m_progLambert.setLightDirection(light);
    m_texture.bind(0);
    m_progLambert.settexture(0);
    m_shadowMapFrameBuffer.bindTextureIntoSlot(1,2);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    m_terrain.draw(&m_progLambert);
    m_progParticle.draw(m_particles);

    // mini map
    m_miniMapFrameBuffer.useMe();
    float camRange = 64;
    glm::vec3 camTarget = m_player.mcr_camera.mcr_position * glm::vec3(1,0,1);
    glm::vec3 camPos = camTarget + glm::vec3(0,2,0) - glm::normalize(m_player.mcr_camera.getforward() * glm::vec3(1,0,1));
    glm::mat4 camView = glm::lookAt(camPos, camTarget, glm::vec3(0,1,0) + m_player.mcr_camera.getforward());
    glm::mat4 camProj = glm::ortho(-camRange, camRange, -camRange, camRange, -260.f, 1.f);
    glm::mat4 camViewProj = camProj * camView;
    m_progLambert.setViewProjMatrix(camViewProj);
    m_progLambert.setModelMatrix(glm::mat4());

    lightDir = glm::vec3(0,1,0);
    lightTarget = m_player.mcr_camera.mcr_position;
        lightTarget.x = Terrain::clampToCoordinate(lightTarget.x, TERRAIN_SIZE) + 0.5 * TERRAIN_SIZE;
        lightTarget.y = 0;
        lightTarget.z = Terrain::clampToCoordinate(lightTarget.z, TERRAIN_SIZE) + 0.5 * TERRAIN_SIZE;
    lightPos = lightTarget + lightDir;
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0,1,1));
    lightProj = glm::ortho(-shadowRange, shadowRange, -shadowRange, shadowRange, -300.f, 32.f);
    lightViewProj = lightProj * lightView;
    m_progLambert.setLightViewProj(lightViewProj);
    m_progLambert.setLightDirection(light);
    m_texture.bind(0);
    m_progLambert.settexture(0);
    m_shadowMapFrameBuffer.bindTextureIntoSlot(1,2);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    m_terrain.draw(&m_progLambert);

    // Render the frame buffer as a texture on a screen-size quad
    m_playerFrameBuffer.useDefault();
    GLCall(glClearColor(.5f, .5f, .5f, 0));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_BACK));
    GLCall(glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio()));
    m_progPostprocessCurrent.useMe();
    m_progPostprocessCurrent.setModelMatrix(glm::mat4(1));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCall(glUniform1i(m_progPostprocessCurrent.unifSamplerDepth2D,2));
    GLCall(glUniform1i(m_progPostprocessCurrent.unifSampler2D, 1));
    GLCall(glUniform1i(m_progPostprocessCurrent.unifSamplerSky2D,3));
    m_playerFrameBuffer.bindTextureIntoSlot(1,2);
    m_skyFrameBuffer.bindTextureIntoSlot(3,4);
    m_miniMapFrameBuffer.bindTextureIntoSlot(5,6);
    m_progPostprocessCurrent.draw(m_geomQuad);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)

void MyGL::putblock(){
    float out_dist;
    glm::ivec3 outblockhit;
    glm::vec3 outblockhitmiddlepoint;
    glm::vec3 distance;
    glm::vec3 rayorigin=m_player.mcr_camera.mcr_position;
    glm::vec3 raydirection=glm::normalize(m_player.mcr_camera.getforward());
    bool hit=m_player.gridMarch(rayorigin,raydirection*3.f,m_terrain,&out_dist,&outblockhit);
    if(hit==true){
        glm::vec3 intersectpoint=rayorigin+raydirection*out_dist;
        for(int i=0;i<3;i++){
            outblockhitmiddlepoint[i]=outblockhit[i]+0.5f;
        }

        distance=intersectpoint-outblockhitmiddlepoint;
        if(std::abs(distance[1])>std::abs(distance[2])&&std::abs(distance[1])>std::abs(distance[0])){
            if(distance[1]>=0){
                m_terrain.setBlockAt(outblockhit[0], outblockhit[1]+1,outblockhit[2], m_player.putblock);
            }
            else{
                 m_terrain.setBlockAt(outblockhit[0], outblockhit[1]-1,outblockhit[2], m_player.putblock);
            }
        }
        if(std::abs(distance[0])>std::abs(distance[1])&&std::abs(distance[0])>std::abs(distance[2])){
            if(distance[0]>=0){
                m_terrain.setBlockAt(outblockhit[0]+1, outblockhit[1],outblockhit[2], m_player.putblock);
            }
            else{
                 m_terrain.setBlockAt(outblockhit[0]-1, outblockhit[1],outblockhit[2], m_player.putblock);
            }
        }
        if(std::abs(distance[2])>std::abs(distance[1])&&std::abs(distance[2])>std::abs(distance[0])){
            if(distance[2]>=0){
                 m_terrain.setBlockAt(outblockhit[0], outblockhit[1],outblockhit[2]+1, m_player.putblock);
            }
            else{
                 m_terrain.setBlockAt(outblockhit[0], outblockhit[1],outblockhit[2]-1, m_player.putblock);
            }
        }

    }
}
void MyGL::deleteblock(){
    float out_dist;
    glm::ivec3 outblockhit;
    glm::vec3 rayorigin=m_player.mcr_camera.mcr_position;
    glm::vec3 raydirection=glm::normalize(m_player.mcr_camera.getforward())*3.f;
    bool hit=m_player.gridMarch(rayorigin,raydirection,m_terrain,&out_dist,&outblockhit);
    if(hit==true){
        m_terrain.setBlockAt(outblockhit[0],outblockhit[1],outblockhit[2],EMPTY);

    }

}

void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    }else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed=true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed=true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed=true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed=true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed=true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed=true;
    } else if(e->key()==Qt::Key_F){
        m_player.flightmode=!m_player.flightmode;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
        m_player.jumpmode=true;
    } else if (e->key()==Qt::Key_1){
        m_player.putblock=GRASS;
    } else if (e->key()==Qt::Key_2){
        m_player.putblock=WATERBLOCK;
    }else if (e->key()==Qt::Key_3){
        m_player.putblock=LAVABLOCK;
    }else if (e->key()==Qt::Key_7){
        weather = CLEAR;
    }else if (e->key()==Qt::Key_8){
        weather = RAIN;
    }else if (e->key()==Qt::Key_9){
        weather = SNOW;
    }
}


void MyGL::keyReleaseEvent(QKeyEvent *e){
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed=false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed=false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed=false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed=false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed=false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed=false;
    }else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
        m_player.jumpmode=false;
    }

}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    float sensitivity=0.5f;
    float mousexoffset=m_inputs.lastmouseX - e->x();
    float mouseyoffset=m_inputs.lastmouseY- e->y();
    m_inputs.mouseX = mousexoffset *sensitivity;
    m_inputs.mouseY = mouseyoffset *sensitivity;
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton){
        putblock();
    }
    if(e->button()==Qt::LeftButton){
        deleteblock();
    }
}
