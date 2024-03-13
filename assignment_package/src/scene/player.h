#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"

class Player : public Entity {

private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    const Terrain &mcr_terrain;
    float pitch;
    float accelarationnumber;
    void groundcheckandjump(glm::vec3& movevector,const Terrain &terrain,float dT);
    void computemovevectorforcollisioncase(glm::vec3& movevector,const Terrain &terrain);
    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain,InputBundle &inputs);


public:

    enum Status {
        STATUS_DEFAULT, STATUS_WATER, STATUS_LAVA
    };
    Status currentStatus;
    // Readonly public reference to our camera
    // for easy access from MyGL

    //block type used to change the block that player can put
    BlockType putblock=GRASS;
    const Camera& mcr_camera;
    //varibale to track if the player in the flightmode;
    bool flightmode;
    //variable used to check if key_space has been pressed;
    bool jumpmode;
    bool groundcheck;


    Player(glm::vec3 pos, const Terrain &terrain);
    virtual ~Player() override;
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);
    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;
};
