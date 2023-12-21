//SDL
#include <SDL2/SDL.h>

// GL extension wrangler
#include <GL/glew.h>

//GL math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Local libraries
#include "shared_globals.hpp"

// class header
#include "flycamera.hpp"

const float CAMERA_SPEED {5.f};
const float MAX_PITCH {89.f};
const float MIN_PITCH {-89.f};
const float MAX_FOV {80.f};
const float MIN_FOV {40.f};

FlyCamera::FlyCamera(): 
    FlyCamera {
        glm::vec3(0.f, 0.f, 0.f), 
        0.f, 0.f,
        45.f
    }
{}

FlyCamera::FlyCamera(glm::vec3 position, float yaw, float pitch, float fov): 
    mFOV {fov},
    mPosition {position},
    mOrientation {
        yaw, 
        pitch
    }
{
    updateYaw(0.f);
    updatePitch(0.f);
    updateFOV(0.f);
    setActive(mActive);
}

void FlyCamera::update(float deltaTime) {
    if(!mActive) return;

    //Calculate new orientation and position vectors
    const glm::vec3 tempUp {0.f, 1.f, 0.f};
    glm::vec3 cameraDirection {
        cos(glm::radians(mOrientation.y)) * sin(glm::radians(mOrientation.x)),
        sin(glm::radians(mOrientation.y)),
        cos(glm::radians(mOrientation.y)) * (-cos(glm::radians(mOrientation.x)))
    };
    mPosition += deltaTime * mVelocity.z * cameraDirection;
    mPosition += deltaTime * mVelocity.x * glm::normalize(glm::cross(cameraDirection, tempUp));
}

glm::vec3 FlyCamera::getPosition() {
    return mPosition;
}
glm::vec3 FlyCamera::getForward() {
    glm::vec3 cameraDirection {
        cos(glm::radians(mOrientation.y)) * sin(glm::radians(mOrientation.x)),
        sin(glm::radians(mOrientation.y)),
        cos(glm::radians(mOrientation.y)) * (-cos(glm::radians(mOrientation.x)))
    };
    return cameraDirection;
}

glm::mat4 FlyCamera::getViewMatrix(){
    const glm::vec3 tempUp {0.f, 1.f, 0.f};
    glm::vec3 cameraDirection { getForward() };
    glm::mat4 viewMatrix {
        glm::lookAt(
            mPosition,
            mPosition + cameraDirection,
            tempUp
        )
    };
    return viewMatrix;
}

glm::mat4 FlyCamera::getProjectionMatrix(){
    glm::mat4 projectionMatrix {
        glm::perspective(
            static_cast<float>(glm::radians(mFOV)),
            static_cast<float>(gWindowWidth)/static_cast<float>(gWindowHeight),
            0.1f,
            100.f
        )
    };
    return projectionMatrix;
}

void FlyCamera::processInput(SDL_Event* event) {
    if(!event) return;

    //Toggle active if 1 is pressed
    if(
        event->type == SDL_KEYUP 
        && event->key.keysym.sym == SDLK_1
    ) {
        setActive(!mActive);
        return;
    }

    // or exit early if this camera is disabled
    if (!mActive) return;

    //Handle mouse movement
    if(event->type == SDL_MOUSEMOTION) {
        float dX { static_cast<float>(event->motion.xrel) };
        float dY { static_cast<float>(event->motion.yrel) };
        updateYaw(dX);
        updatePitch(dY);
        return;
    }
    if(event->type == SDL_MOUSEWHEEL){
        float dFOV { -static_cast<float>(event->wheel.y) };
        updateFOV(dFOV);
        return;
    }

    //Handle keydown
    if(event->type == SDL_KEYDOWN){
        switch(event->key.keysym.sym) {
            case SDLK_w:
                mVelocity.z = CAMERA_SPEED;
            break;
            case SDLK_a:
                mVelocity.x = -CAMERA_SPEED;
            break;
            case SDLK_s:
                mVelocity.z = -CAMERA_SPEED;
            break;
            case SDLK_d:
                mVelocity.x = CAMERA_SPEED;
            break;
        }
        return;
    }

    // Handle keyup
    if(event->type == SDL_KEYUP) {
        switch(event->key.keysym.sym) {
            // handle camera movements
            case SDLK_w:
            case SDLK_s:
                mVelocity.z = 0.f;
            break;
            case SDLK_a:
            case SDLK_d:
                mVelocity.x = 0.f;
            break;
        }
        return;
    }
}

void FlyCamera::setActive(bool active) {
    mActive = active;
    SDL_SetRelativeMouseMode(mActive? SDL_TRUE: SDL_FALSE);
    SDL_CaptureMouse(mActive? SDL_TRUE: SDL_FALSE);
    SDL_ShowCursor(mActive? SDL_FALSE: SDL_TRUE);
}

void FlyCamera::setLookSensitivity(float lookSensitivity) {
    mLookSensitivity = lookSensitivity;
}

void FlyCamera::setZoomSensitivity(float zoomSensitivity) {
    mZoomSensitivity = zoomSensitivity;
}

void FlyCamera::updatePitch(float dPitch) {
    mOrientation.y += mLookSensitivity * (-dPitch);
    if(mOrientation.y > MAX_PITCH) 
        mOrientation.y = MAX_PITCH;
    else if(mOrientation.y < MIN_PITCH) 
        mOrientation.y = MIN_PITCH;
}

void FlyCamera::updateYaw(float dYaw) {
    mOrientation.x += mLookSensitivity * dYaw;
    mOrientation.x = (
        (180.f + mOrientation.x)
        - (static_cast<int>((180.f + mOrientation.x)/360.f) * 360.f)
        - 180.f
    );
}

void FlyCamera::updateFOV(float dFOV) {
    mFOV += mZoomSensitivity + dFOV;
    if(mFOV > MAX_FOV) mFOV = MAX_FOV;
    else if(mFOV < MIN_FOV) mFOV = MIN_FOV;
}
