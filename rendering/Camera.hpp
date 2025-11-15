#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 target;
    float distance;
    float pitch;
    float yaw;
    float focalLenth;

    Camera(glm::vec3 target, float distance, float pitch, float yaw, float focalLenth = 45.0f) :
        target(target), distance(distance), pitch(pitch), yaw(yaw), focalLenth(focalLenth) {}

    glm::vec3 getPosition() const;

    glm::vec3 getLookAtDirection();
    glm::vec3 getForwardDirection();
    glm::vec3 getRightDirection();
    glm::mat4 getViewMatrix();
};

#endif