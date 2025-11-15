#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Camera::getPosition() const {
    float x = target.x + distance * cos(pitch) * sin(yaw);
    float y = target.y + distance * sin(pitch);
    float z = target.z + distance * cos(pitch) * cos(yaw);
    return glm::vec3(x, y, z);
}

glm::vec3 Camera::getLookAtDirection() {
    return glm::normalize(target - getPosition());
}

glm::vec3 Camera::getForwardDirection() {
    // Direction vers le target dans le plan horizontal
    glm::vec3 forward = glm::normalize(glm::vec3(
        target.x - getPosition().x,
        0,
        target.z - getPosition().z
    ));
    return forward;
}

glm::vec3 Camera::getRightDirection() {
    glm::vec3 forward = getForwardDirection();
    return glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
}