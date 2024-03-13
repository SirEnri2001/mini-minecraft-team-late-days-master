#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <glm_includes.h>
namespace ShadowMap
{
glm::mat4 ViewProjection(glm::vec3 lightDir, glm::vec3 playerPos);

}



#endif // SHADOWMAP_H
