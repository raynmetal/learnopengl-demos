#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shared_globals.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "flycamera.hpp"
#include "shader.hpp"
#include "light.hpp"

//Initialize camera variables
bool gWireframeMode { false };

float gDeltaTime {0.f};

extern const int gWindowWidth {800};
extern const int gWindowHeight {600};
SDL_Window* gWindow {nullptr};
FlyCamera* gCamera {nullptr};

bool init(SDL_Window*& window, SDL_GLContext& context);
void close(SDL_GLContext& context);
void processInput(SDL_Event* event);

int main(int argc, char* argv[]) {
    SDL_GLContext context {};

    // Initialize SDL context
    if(!init(gWindow, context)) {
        close(context);
        return 1;
    }

    //Load object shader program
    Shader objectShader {"shaders/vertex.vs", "shaders/object_fragment.fs"};
    if(!objectShader.getBuildSuccess() /*|| !lightSourceShader.getBuildSuccess()*/) {
        std::cout << "Oops, object shader failed to load" << std::endl;
        close(context);
        return 1;
    }

    // Load light source shader program
    // Shader lightSourceShader {"shaders/vertex.vs", "shaders/lightsource_fragment.fs"};

    //Set clear colour to a dark green-blueish colour
    glClearColor(.2f, .3f, .3f, 1.f);

    //Use the object shader we loaded as our current shader program
    objectShader.use();
    //Load our model
    // Model backpack {"media/backpack.obj", objectShader};

    //Set up a VAO for a single upright square
    GLuint quadVAO {};
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
        std::vector<GLfloat> quadVertices {
            //bottom left
            -0.5f, 0.f, 0.f, //position (xyz)
                0.f, 1.f, // texture coordinate (st)
                0.f, 0.f, 1.f, //normal (xyz)
            //bottom right
            0.5f, 0.f, 0.f,
                1.f, 1.f,
                0.f, 0.f, 1.f,
            //top right
            0.5f, 1.f, 0.f,
                1.f, 0.f,
                0.f, 0.f, 1.f,
            //top left
            -0.5f, 1.f, 0.f,
                0.f, 0.f,
                0.f, 0.f, 1.f
        };
        std::vector<GLuint> quadElements {
            0, 1, 2, // bottom right triangle
            0, 2, 3 // top left triangle
        };

        // Send vertex buffer data
        GLuint quadVBO {};
        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(
            GL_ARRAY_BUFFER, 
            quadVertices.size() * sizeof(float),
            &quadVertices[0],
            GL_STATIC_DRAW
        );
        GLuint quadEBO {};
        glGenBuffers(1, &quadEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            quadElements.size() * sizeof(GLuint),
            &quadElements[0],
            GL_STATIC_DRAW
        );

        objectShader.enableAttribArray("position");
        objectShader.setAttribPointerF("position", 3, 8, 0);

        objectShader.enableAttribArray("textureCoord");
        objectShader.setAttribPointerF("textureCoord", 2, 8, 3);

        objectShader.enableAttribArray("normal");
        objectShader.setAttribPointerF("normal", 3, 8, 5);
    glBindVertexArray(0);

    Texture grassTexture {"media/grass.png", "texture_diffuse"};

    //Set up light source properties
    glm::vec3 lightSourcePosition {2.f, 2.f, 2.f};
    glm::vec3 lightAmbient {.2f, .2f, .2f};
    glm::vec3 lightDiffuse {.5f, .5f, .5f};
    glm::vec3 lightSpecular {1.f, 1.f, 1.f};
    float lightInnerAngle {18.f};
    float lightOuterAngle {20.f};
    float lightLinear {.09f};
    float lightQuadratic {.032f};

    //Create one spotlight
    Light spotLight { 
        makeSpotLight(
            lightSourcePosition, glm::vec3(0.f), lightInnerAngle,
            lightOuterAngle, lightDiffuse, lightSpecular,
            lightAmbient, lightLinear, lightQuadratic
        )
    };
    objectShader.setLight("lights[0]", spotLight);
    // Create 4 point lights around (0, 4, 2)
    glm::vec3 pointLightPositions[4];
    for(int i {0}; i < 4; ++i) {
        pointLightPositions[i] = glm::vec3 (
            5.f * sin(glm::radians(360.f/4.f * i)),
            4.f,
            2.f + 5.f * cos(glm::radians(360.f/4.f * i))
        );
        glm::vec3 pos { pointLightPositions[i] };
        Light pointLight {
            makePointLight(pos,
                lightDiffuse, lightSpecular, lightAmbient,
                lightLinear, lightQuadratic)
        };
        std::stringstream lUniform{};
        lUniform << "lights[" << (1+i) << "]";
        objectShader.setLight(lUniform.str(), pointLight);
    }
    Light directionalLight {
        makeDirectionalLight(glm::vec3(2.f, -3.f, 2.f), lightDiffuse, lightSpecular, lightAmbient)
    };
    objectShader.setLight("lights[5]", directionalLight);

    //Set up material properties
    GLint materialShine {32};
    objectShader.setInt("material.shine", materialShine);

    //Near and far plane depths
    objectShader.setFloat("nearDepth", 1.f);
    objectShader.setFloat("farDepth", 50.f);

    // //Render an instance of the cube at the following position
    // glm::vec3 cubePositions[] {
    //     glm::vec3(0.f, 0.f, -2.f),
    // };

    std::vector<glm::vec3> vegetationPositions {
        {-1.5f, 0.f, -.48f},
        {1.5f, 0.f, .51f},
        {0.f, 0.f, .7f},
        {-.3f, 0.f, -2.3f},
        {.5f, 0.f, -.6f}
    };

    //Timing related variables
    uint64_t lastFrame {SDL_GetTicks64()}; // time of last frame

    //Initialize camera (and view and projection matrices)
    gCamera = new FlyCamera{};
    gCamera->setActive(true); 
    gCamera->update(0.f);
    gCamera->setActive(false);
    glm::vec3 cameraPosition {gCamera->getPosition()};

    //Main event loop
    SDL_Event event;
    bool quit {false};
    while(true) {
        //Check SDL event queue for any events, process them
        while(SDL_PollEvent(&event)) {
            //Handle exit events
            if(
                event.type == SDL_QUIT 
                || (
                    event.type == SDL_KEYUP 
                    && event.key.keysym.sym == SDLK_ESCAPE
                )
            ) {
                quit = true;
                break;
            } 

            //Handle window resizing events
            else if(event.type == SDL_WINDOWEVENT) {
                switch(event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        glViewport(
                            0, 0,
                            event.window.data1, event.window.data2
                        );
                    break;
                }
            }
            else processInput(&event);

            if(gWireframeMode)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else 
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if(quit) break;

        //Update time related variables
        uint64_t currentFrame {SDL_GetTicks64()};
        gDeltaTime = static_cast<float>(currentFrame - lastFrame)/1000.f;
        lastFrame = currentFrame;

        //Update the camera and related matrices
        gCamera->update(gDeltaTime);
        glm::mat4 projectionTransform {gCamera->getProjectionMatrix()};
        glm::mat4 viewTransform {gCamera->getViewMatrix()};
        cameraPosition = gCamera->getPosition();

        //Clear colour, stencil, and depth buffers before each render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw vegetation
        objectShader.use();
        objectShader.setMat4("projection", projectionTransform);
        objectShader.setMat4("view", viewTransform);
        objectShader.setVec3("eyePos", cameraPosition);
        objectShader.setVec3("lights[0].position", cameraPosition);
        objectShader.setVec3("lights[0].direction", gCamera->getForward());
        glActiveTexture(GL_TEXTURE0);
        objectShader.setInt("material.texture_diffuse1", 0);
        grassTexture.bindTexture(true);
        for(glm::vec3 position : vegetationPositions) {
            glm::mat4 model { glm::translate(glm::mat4(1.f), position) };
            glm::mat4 normal { glm::transpose(glm::inverse(model)) };
            objectShader.setMat4("model", model);
            objectShader.setMat4("normalMat", normal);
            glBindVertexArray(quadVAO);
                glDrawElements(GL_TRIANGLES, quadElements.size(), GL_UNSIGNED_INT, static_cast<void*>(0));
            glBindVertexArray(0);
        }
        grassTexture.bindTexture(false);

        // //Draw objects
        // objectShader.use();
        // objectShader.setMat4("projection", projectionTransform);
        // objectShader.setMat4("view", viewTransform);
        // objectShader.setVec3("eyePos", cameraPosition);
        // objectShader.setVec3("lights[0].position", cameraPosition);
        // objectShader.setVec3("lights[0].direction", gCamera->getForward());
        // for(glm::vec3 position : cubePositions) {
        //     // The Model matrix transforms a single object's vertices
        //     // to its location, orientation, shear, and size, in the 
        //     // world space
        //     float angle {20.f * position.z};
        //     glm::mat4 model { glm::translate(glm::mat4(1.f), position) };
        //     model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));
        //     glm::mat4 normal { glm::transpose(glm::inverse(model)) };
        //     objectShader.setMat4("model", model);
        //     objectShader.setMat4("normalMat", normal);
        //     //Draw
        //     backpack.Draw(objectShader);
        // }

        //Update screen
        SDL_GL_SwapWindow(gWindow);
    }

    // de-allocate resources
    delete gCamera;
    gCamera = nullptr;

    close(context);
    return 0;
}

void processInput(SDL_Event* event) {
    gCamera->processInput(event);
}

bool init(SDL_Window*& window, SDL_GLContext& context) {
    //Initialize SDL subsystems
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    //Specify that we want a forward compatible OpenGL 3.3 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // creating a stencil buffer

    // Create an OpenGL window and context with SDL
    window = SDL_CreateWindow("OpenGL", 100, 100, gWindowWidth, gWindowHeight,  SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);
    if(!window || !context) return false;

    //Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    //Set up viewport
    glViewport(0, 0, 800, 600);

    // Enable OpenGL depth testing
    glEnable(GL_DEPTH_TEST);

    //Determines the type of depth function used; GL_LESS
    //is the default one. Fragments are discarded if their
    //depth is greater than the presently stored depth for
    //a given fragment
    glDepthFunc(GL_LESS);

    return true;
}

void close(SDL_GLContext& context){
    //Kill the OpenGL context before quitting
    SDL_GL_DeleteContext(context);

    // Then die
    SDL_Quit();
}
