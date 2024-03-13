#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
    m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),pitch(0.f),accelarationnumber(15.f),mcr_camera(m_camera),
    flightmode(true),jumpmode(false),groundcheck(false)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain,input);
}

void Player::groundcheckandjump(glm::vec3& movevector,const Terrain &terrain,float dT){
    groundcheck=false;
    //get bottom four vertices
    std::vector<glm::vec3> verticesposition;
    for(float x=-0.5f;x<=0.5f;x+=1.0f){
        for(float z=-0.5f;z<=0.5f;z+=1.0f){
           verticesposition.push_back(glm::vec3(m_position.x+x,m_position.y-0.5f,m_position.z+z));
        }
    }
    int size=verticesposition.size();
    for(int i=0;i<size;i++){
        BlockType blockType = terrain.getBlockAt(glm::floor(verticesposition[i]));

        if (blockType!=EMPTY&&blockType!=WATER){
           groundcheck=true;
           m_velocity.y = 0.f;
           movevector.y=0.f;
           break;
        }
    }
    if(groundcheck==true){
        if(jumpmode==true){
           m_velocity.y=10.f;
           movevector.y=m_velocity.y*dT;
        }
    }


}
void Player::computemovevectorforcollisioncase(glm::vec3& movevector,const Terrain &terrain){
    //get 12 vertex position
    std::vector<glm::vec3> verticesposition;
    for(float x=-0.5f;x<=0.5f;x+=1.0f){
        for(float z=-0.5f;z<=0.5f;z+=1.0f){
            for(float y=0.f;y<=2.f;y+=1.0f){
                verticesposition.push_back(glm::vec3(m_position.x+x,m_position.y+y,m_position.z+z));
            }
        }
    }

    glm::vec3 rayxdirection=glm::vec3(movevector.x,0.0f,0.0f);
    glm::vec3 rayydirection=glm::vec3(0.0f,movevector.y,0.0f);
    glm::vec3 rayzdirection=glm::vec3(0.0f,0.0f,movevector.z);
    for(int i=0;i<verticesposition.size();i++){
        glm::vec3 rayorigin=verticesposition[i];
        // test ray for x direction
        float xout_dist;
        glm::ivec3 out_blockhitx;
        bool xdetection=gridMarch(rayorigin,rayxdirection,terrain,&xout_dist,&out_blockhitx);
        // test ray for y direction
        float yout_dist;
        glm::ivec3 out_blockhity;
        bool ydetection=gridMarch(rayorigin,rayydirection,terrain,&yout_dist,&out_blockhity);
        //test ray for z direction
        float zout_dist;
        glm::ivec3 out_blockhitz;
        bool zdetection=gridMarch(rayorigin,rayzdirection,terrain,&zout_dist,&out_blockhitz);

        //change movevector when x ray hit object
        if(xdetection){
            if(xout_dist<std::abs(movevector.x)){
                if(movevector.x<0.f){
                    movevector.x=-xout_dist+0.001f;
                }
                else{
                    movevector.x=xout_dist-0.001f;
                }
                m_velocity.x = 0.f;
            }
        }
        // change movevector when y ray hit object
        if(ydetection){
            if(yout_dist<std::abs(movevector.x)){
                if(movevector.y<0.f){
                    movevector.y=-yout_dist+0.001f;
                }
                else{
                    movevector.y=yout_dist-0.001f;
                }
                m_velocity.y = 0.f;
            }
        }
        // change movevector when z ray hitobject
        if(zdetection){
            if(zout_dist<std::abs(movevector.z)){
                if(movevector.z<0.f){
                    movevector.z=-zout_dist+0.001f;
                }
                else{
                    movevector.z=zout_dist-0.001f;
                }
                m_velocity.z = 0.f;
            }
        }

    }


}
void Player::processInputs(InputBundle &inputs) {
    //rotate up axis first
   rotateOnUpGlobal(inputs.mouseX);
    //rotate local right axis
     pitch = glm::clamp(pitch+inputs.mouseY, -89.99f, 89.99f);
   if(pitch<89.99f&&pitch>-89.99f){
         rotateOnRightLocal(inputs.mouseY);
   }
    m_acceleration = {0.f, 0.f, 0.f};
    inputs.mouseX = 0.f;
    inputs.mouseY = 0.f;
    //we need player to move up alone the world up axis
    glm::vec3 upforacceleration=glm::vec3(0.f, 1.f, 0.f);

    //calculate acceleration based on state of inputs
    float frictionFactor = 0.f;
    if (flightmode) {
        if (inputs.wPressed == true) {
            m_acceleration += accelarationnumber * m_forward;
        }
        if (inputs.sPressed == true) {
            m_acceleration -= accelarationnumber * m_forward;
        }
        if (inputs.dPressed == true) {
            m_acceleration += accelarationnumber * m_right;
        }
        if (inputs.aPressed == true) {
             m_acceleration -= accelarationnumber * m_right;
        }
        if (inputs.ePressed == true) {
            m_acceleration += accelarationnumber * upforacceleration;
        }
        if (inputs.qPressed == true) {
            m_acceleration -= accelarationnumber * upforacceleration;
        }
        frictionFactor = 0.99f;
    }
    else if (mcr_terrain.getBlockAt(mcr_position.x, mcr_position.y, mcr_position.z) == WATER) {
        jumpmode = false;
        float damper = 0.67;
        if (inputs.wPressed == true) {
            m_acceleration += damper*accelarationnumber * m_forward;
        }
        if (inputs.sPressed == true) {
            m_acceleration -= damper*accelarationnumber * m_forward;
        }
        if (inputs.dPressed == true) {
            m_acceleration += damper*accelarationnumber * m_right;
        }
        if (inputs.aPressed == true) {
            m_acceleration -= damper*accelarationnumber * m_right;
        }
        if (inputs.spacePressed == true) {
            // m_position += glm::vec3(0, 0.1, 0);
            m_acceleration += damper*accelarationnumber * upforacceleration * 2.f;
        }
        frictionFactor = 0.9f;
    }
    else if (!flightmode) {

        glm::vec3 xzVelocity;
        if (inputs.wPressed == true) {
            glm::vec3 m_forwardforunflightmode=glm::vec3(m_forward.x,0.0f,m_forward.z);
            m_forwardforunflightmode=glm::normalize(m_forwardforunflightmode);
            xzVelocity = accelarationnumber * m_forwardforunflightmode;
            // m_acceleration += accelarationnumber * m_forwardforunflightmode;
        }
        if (inputs.sPressed == true) {
            glm::vec3 m_forwardforunflightmode=glm::vec3(m_forward.x,0.0f,m_forward.z);
            m_forwardforunflightmode=glm::normalize(m_forwardforunflightmode);
            xzVelocity = accelarationnumber * -m_forwardforunflightmode;
            //m_acceleration -= accelarationnumber * m_forwardforunflightmode;
        }
        if (inputs.dPressed == true) {
            glm::vec3 m_rightforunflightmode=glm::vec3(m_right.x,0.0f,m_right.z);
            m_rightforunflightmode=glm::normalize(m_rightforunflightmode);
            xzVelocity = accelarationnumber * m_rightforunflightmode;
            //m_acceleration += accelarationnumber * m_rightforunflightmode;
        }
        if (inputs.aPressed == true) {
            glm::vec3 m_rightforunflightmode=glm::vec3(m_right.x,0.0f,m_right.z);
            m_rightforunflightmode=glm::normalize(m_rightforunflightmode);
            xzVelocity = accelarationnumber * -m_rightforunflightmode;
            //m_acceleration -= accelarationnumber * m_rightforunflightmode;
        }
        m_velocity.x = xzVelocity.x;
        m_velocity.z = xzVelocity.z;

        frictionFactor = 0.9f;
    }
    m_acceleration += -m_velocity * frictionFactor;

    // gravity
    if (!flightmode) {
        glm::vec3 m_downwardforunflightmode=glm::normalize(glm::vec3(0.0f,-20.f,0.0f));
        m_acceleration += accelarationnumber * m_downwardforunflightmode;
    }

}

void Player::computePhysics(float dT, const Terrain &terrain,InputBundle &inputs) {
    const float maxVelocity = 5;
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    if(flightmode==true){
        // m_velocity=glm::vec3(0.0f,0.0f,0.0f);

        m_velocity+=m_acceleration*dT;
        glm::clamp(m_velocity, glm::vec3(-maxVelocity), glm::vec3(maxVelocity));
        glm::vec3 movevector = m_velocity * dT;
        moveAlongVector(movevector);

    }

    if(flightmode==false){
        // m_velocity=glm::vec3(0.0f,0.0f,0.0f);
        if (mcr_terrain.getBlockAt(mcr_position.x, mcr_position.y, mcr_position.z) == WATER&&inputs.wPressed ==false&&inputs.sPressed == false&&inputs.dPressed == false&&inputs.aPressed == false){
            m_velocity=0.5f*glm::normalize(m_forward);
        }
        m_velocity+=m_acceleration*dT;
        glm::vec3 movevector = m_velocity * dT;
        groundcheckandjump(movevector,terrain,dT);
        computemovevectorforcollisioncase(movevector,terrain);
        moveAlongVector(movevector);

     }

}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm:: ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY&&cellType != WATER) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }

    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}


void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
