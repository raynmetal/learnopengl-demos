#include <glm/glm.hpp>

#include "light.hpp"

Light makeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient){
    return Light{
        .mType { Light::directional },
        .mDirection { direction },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {0.f},
        .mQuadratic {0.f},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light makePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst){
    return Light{
        .mType { Light::point },
        .mPosition { position },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {linearConst},
        .mQuadratic {quadraticConst},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light makeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst) {
    return Light{
        .mType { Light::spot },
        .mPosition { position },
        .mDirection { direction },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {linearConst},
        .mQuadratic {quadraticConst},
        .mCosCutoffInner {glm::cos(glm::radians(innerAngle))},
        .mCosCutoffOuter {glm::cos(glm::radians(outerAngle))}
    };
}


