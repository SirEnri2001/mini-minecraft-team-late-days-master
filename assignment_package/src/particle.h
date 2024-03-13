#ifndef PARTICLE_H
#define PARTICLE_H
#include "drawable.h"

class Particle : public Drawable
{
    std::unique_ptr<std::array<glm::vec4, 200000>> particles;
    OpenGLContext* context;
public:
    Particle(OpenGLContext*);
    void createVBOdata() override;
    void update();
    glm::vec4 spawnPos;
    GLenum drawMode() override;
};

#endif // PARTICLE_H
