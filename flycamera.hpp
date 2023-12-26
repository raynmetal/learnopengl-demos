#ifndef ZOFLYCAM_H
#define ZOFLYCAM_H

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class FlyCamera {
public:
    FlyCamera();
    FlyCamera(glm::vec3 position, float yaw, float pitch, float fov);

    void update(float deltaTime);
    void processInput(SDL_Event* event);

    glm::vec3 getPosition();
    glm::vec3 getForward();
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    void setActive(bool active);
    void setLookSensitivity(float lookSensitivity);
    void setZoomSensitivity(float zoomSensitivity);

private:
    void updateYaw(float dYaw);
    void updatePitch(float dPitch);
    void updateFOV(float dFOV);

    bool mActive { false };
    float mFOV { 45.f };
    float mLookSensitivity { 0.1f };
    float mZoomSensitivity { 1.5f };

    glm::vec3 mPosition; //position in world units
    glm::vec2 mOrientation; // pitch and yaw, in degrees
    glm::vec3 mVelocity;
};

#endif
