#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shared_globals.hpp"
#include "flycamera.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utility.hpp"

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

    //Load shader program
    Shader objectShader {"shaders/vertex.vs", "shaders/fragment.fs"};
    Shader lightSourceShader {"shaders/vertex.vs", "shaders/lightsource_fragment.fs"};
    if(!objectShader.getBuildSuccess() || !lightSourceShader.getBuildSuccess()) {
        std::cout << "Oops, one of our shaders failed to load" << std::endl;
        close(context);
        return 1;
    }

    //Load first texture into texture unit 0
    glActiveTexture(GL_TEXTURE0);
    Texture txtr1 { "media/container2.png" };
    if(!txtr1.getTextureID()) {
        std::cout << "Oops, our texture failed to load" << std::endl;
        close(context);
        return 1;
    }
    txtr1.bindTexture(true);
    glActiveTexture(GL_TEXTURE1);
    Texture txtr2 { "media/container2_specular.png" };
    if(!txtr2.getTextureID()) {
        std::cout << "Oops, our specular map texture failed to load" << std::endl;
        close(context);
        return 1;
    }
    txtr2.bindTexture(true);

    glm::vec3 diag {glm::normalize(glm::vec3(1.f,1.f,1.f))};
    //Set up a polygon to draw; here, a triangle
    float vertices[] {
        -.5f, .5f, .5f, // topleft front
            1.f, 0.f, 0.f, //(red)
            0.f, 1.f, // [sample] texture top left
            -diag.x, diag.y, diag.z, // normal
        .5f, .5f, .5f, // topright front
            1.f, 1.f, 0.f, // (yellow)
            1.f, 1.f, // texture top right
            diag.x, diag.y, diag.z,
        .5f, -.5f, .5f, // bottom right front
            0.f, 1.f, 0.f, //(green)
            1.f, 0.f, // texture bottom right
            diag.x, -diag.y, diag.z,
        -.5f, -.5f, .5f, // bottom  left front
            0.f, 0.f, 1.f, //(blue)
            0.f, 0.f, // texture bottom left
            -diag.x, -diag.y, diag.z,
        -.5f, -.5f, -.5f, // bottom left back
            0.f, 0.f, 0.f,
            0.f, 1.f,
            -diag.x, -diag.y, -diag.z,
        .5f, -.5f, -.5f, // bottom right back
            0.f, 0.f, 0.f,
            1.f, 1.f,
            diag.x, -diag.y, -diag.z,
        .5f, .5f, -.5f, // top right back
            0.f, 0.f, 0.f,
            1.f, 0.f,
            diag.x, diag.y, -diag.z,
        -.5f, .5f, -.5f, // top left back
            0.f, 0.f, 0.f,
            0.f, 0.f,
            -diag.x, diag.y, -diag.z
    };

    // Set up element buffer
    GLuint elements[] {
        // front face
        0, 1, 2, 
        0, 2, 3,
        // bottom face
        2, 3, 4,
        2, 4, 5,
        //back face
        4, 5, 6,
        4, 6, 7,
        //top face
        6, 7, 0,
        6, 0, 1,
        // right face
        6, 1, 2,
        6, 2, 5,
        // left face
        0, 7, 4,
        0, 4, 3
    };

    // Set up our vertex buffer
    GLuint vbo {};
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,  // the type of data we're sending
            sizeof(vertices), // length of data being sent, in bytes
            vertices, // the (CPU) memory being copied from (to the GPU memory)
            GL_STATIC_DRAW // A hint as to how often this data will be overwritten
        );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set up our element buffer
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, // type of data we're sending
            sizeof(elements),  // length of data being sent, in bytes
            elements, // the (CPU) memory being copied from
            GL_STATIC_DRAW // A hint as to how often this data will be overwritten
        );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Set up our vertex array object. A set of buffer and pointer
    // bindings used for a particular set of draw calls
    objectShader.use();
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        //Enable vertex pointer (one for position, another for colour)
        objectShader.enableAttribArray("position");
        objectShader.enableAttribArray("color");
        objectShader.enableAttribArray("textureCoord");
        objectShader.enableAttribArray("normal");

        //Specify which buffer to use for vertices, elements
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        //Define the format of each vertex position, color, texture coordinates, and normals
        objectShader.setAttribPointerF("position", 3, 11, 0);
        objectShader.setAttribPointerF("color", 3, 11, 3);
        objectShader.setAttribPointerF("textureCoord", 2, 11, 6);
        objectShader.setAttribPointerF("normal", 3, 11, 8);
    glBindVertexArray(0);

    // Set up attribute pointers for the light source shader
    lightSourceShader.use();
    GLuint lightSourceVao {};
    glGenVertexArrays(1, &lightSourceVao);
    glBindVertexArray(lightSourceVao);
        lightSourceShader.enableAttribArray("position");
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        lightSourceShader.setAttribPointerF("position", 3, 11, 0);
    glBindVertexArray(0);

    //Set clear colour to a dark green-blueish colour
    glClearColor(.2f, .3f, .3f, 1.f);

    //Use the shader we loaded as our shader program
    objectShader.use();

    //Set up light source properties
    glm::vec3 lightSourcePosition {2.f, 2.f, 2.f};
    glm::vec3 lightAmbient {.2f, .2f, .2f};
    glm::vec3 lightDiffuse {.5f, .5f, .5f};
    glm::vec3 lightSpecular {1.f, 1.f, 1.f};
    objectShader.setVec3("light.position", lightSourcePosition);
    objectShader.setVec3("light.ambient", lightAmbient);
    objectShader.setVec3("light.diffuse", lightDiffuse);
    objectShader.setVec3("light.specular", lightSpecular);

    //Set up material properties
    glm::vec3 materialSpecular {.2f, .2f, .2f};
    GLint materialShine {32};
    // --
    glActiveTexture(GL_TEXTURE0);
    txtr1.bindTexture(true);
    objectShader.setInt("material.diffuse", 0);
    // -- 
    glActiveTexture(GL_TEXTURE1);
    txtr2.bindTexture(true);
    objectShader.setInt("material.specular", 1);
    // --
    objectShader.setInt("material.shine", materialShine);

    //Render an instance of the cube at the following position
    glm::vec3 cubePositions[] {
        glm::vec3(0.f, 0.f, -2.f)
    };

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
        lightSourcePosition = glm::vec3(2.85f * sin(static_cast<float>(currentFrame)/1000.f), 2.f, 2.85f * cos(static_cast<float>(currentFrame)/1000.f));
        cameraPosition = gCamera->getPosition();

        //Clear colour and depth buffers before each render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Draw objects
        objectShader.use();
        objectShader.setMat4("projection", projectionTransform);
        objectShader.setMat4("view", viewTransform);
        objectShader.setVec3("light.position", lightSourcePosition);
        objectShader.setVec3("eyePos", cameraPosition);
        for(glm::vec3 position : cubePositions) {
            // The Model matrix transforms a single object's vertices
            // to its location, orientation, shear, and size, in the 
            // world space
            glm::mat4 model {glm::translate(glm::mat4(1.f), position)};
            glm::mat4 normal { glm::transpose(glm::inverse(model)) };
            objectShader.setMat4("model", model);
            objectShader.setMat4("normalMat", normal);
            //Draw
            glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        //Draw light source
        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projectionTransform);
        lightSourceShader.setMat4("view", viewTransform);
        glm::mat4 model {glm::mat4(1.f)};
        model = glm::translate(model, lightSourcePosition);
        model = glm::scale(model, glm::vec3(.3f));
        lightSourceShader.setMat4("model", model);
        //Draw
        glBindVertexArray(lightSourceVao);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //Update screen
        SDL_GL_SwapWindow(gWindow);
    }

    // de-allocate resources
    delete gCamera;
    gCamera = nullptr;
    glDeleteVertexArrays(1, &vao);
    glDeleteVertexArrays(1, &lightSourceVao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

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

    return true;
}

void close(SDL_GLContext& context){
    //Kill the OpenGL context before quitting
    SDL_GL_DeleteContext(context);

    // Then die
    SDL_Quit();
}
