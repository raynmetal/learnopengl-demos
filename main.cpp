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

    //Vertex attrib arrays
    GLint vertexAttrib { glGetAttribLocation(objectShader.getProgramID(), "position") };
    GLint colorAttrib { glGetAttribLocation(objectShader.getProgramID(), "color") };
    GLint textureCoordAttrib{ glGetAttribLocation(objectShader.getProgramID(), "textureCoord") };
    GLint normalAttrib {glGetAttribLocation(objectShader.getProgramID(), "normal")};
    //Model, view, projection matrices
    GLint modelUniform { glGetUniformLocation(objectShader.getProgramID(), "model")};
    GLint normalUniform { glGetUniformLocation(objectShader.getProgramID(), "normalMat")};
    GLint viewUniform { glGetUniformLocation(objectShader.getProgramID(), "view")};
    GLint projectionUniform { glGetUniformLocation(objectShader.getProgramID(), "projection")};
    //Light-related attributes
    GLint eyePositionUniform { glGetUniformLocation(objectShader.getProgramID(), "eyePos")};
    GLint lightPositionUniform { glGetUniformLocation(objectShader.getProgramID(), "lightPos")};
    GLint objectColorUniform {glGetUniformLocation(objectShader.getProgramID(), "objectColor")};
    GLint lightColorUniform { glGetUniformLocation(objectShader.getProgramID(), "lightColor")};
    GLint ambientStrengthUniform { glGetUniformLocation(objectShader.getProgramID(), "ambientStrength")};
    GLint specularStrengthUniform { glGetUniformLocation(objectShader.getProgramID(), "specularStrength")};
    GLint specularShineUniform { glGetUniformLocation(objectShader.getProgramID(), "specularShine")};

    //Load first texture into texture unit 0
    glActiveTexture(GL_TEXTURE0);
    Texture txtr1 { "media/wall.jpg"  };
    if(!txtr1.getTextureID()) {
        std::cout << "Oops, our texture failed to load" << std::endl;
        close(context);
        return 1;
    }
    txtr1.bindTexture(true);

    //Load second texture into texture unit 1
    glActiveTexture(GL_TEXTURE1);
    Texture txtr2 { "media/awesomeface.png"  };
    if(!txtr2.getTextureID()) {
        std::cout << "Oops, our texture failed to load" << std::endl;
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
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        //Enable vertex pointer (one for position, another for colour)
        glEnableVertexAttribArray(vertexAttrib);
        glEnableVertexAttribArray(colorAttrib);
        glEnableVertexAttribArray(textureCoordAttrib);
        glEnableVertexAttribArray(normalAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        //Specify which buffer to use for vertices, elements
        //Define the format of each vertex position in above buffer
        glVertexAttribPointer(
            vertexAttrib, // attrib pointer
            3, // Number of values for a "single" input element
            GL_FLOAT, //The format of each component
            GL_FALSE, //Normalize? in case not floating point
            //Format of the attribute array, in terms of
            // |.....|VERTEX_0|.......||.....|VERTEX_1|.......|
            // |-----| --> Offset
            //       |-----------------------| --> Stride
            //       |--------| --> Single vertex element (size of GL_FLOAT)
            11*sizeof(float), // Stride: number of bytes between each position, with 0 indicating there's no offset to the next element
            reinterpret_cast<void*>(0) // Offset: offset of the first element relative to the start of the array
        );
        //Define the format of each vertex color in above buffer
        glVertexAttribPointer(
            colorAttrib, 
            3, GL_FLOAT, GL_FALSE,
            11*sizeof(float), // Stride
            reinterpret_cast<void*>(3*sizeof(float)) // Offset
        );
        glVertexAttribPointer(
            textureCoordAttrib,
            2, GL_FLOAT, GL_FALSE,
            11*sizeof(float), // Stride
            reinterpret_cast<void*>(6*sizeof(float)) // Offset
        );
        glVertexAttribPointer(
            normalAttrib,
            3, GL_FLOAT, GL_FALSE,
            11*sizeof(float), // Stride
            reinterpret_cast<void*>(8*sizeof(float)) // Offset
        );
    glBindVertexArray(0);

    // Set up attribute pointers for the light source shader
    GLuint lightSourceVao {};
    glGenVertexArrays(1, &lightSourceVao);
    glBindVertexArray(lightSourceVao);
        GLint lightPositionAttribute {glGetAttribLocation(lightSourceShader.getProgramID(), "position")};
        glEnableVertexAttribArray(
            lightPositionAttribute
        );
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glVertexAttribPointer(
            lightPositionAttribute,
            3, GL_FLOAT, GL_FALSE,
            11 * sizeof(float),
            reinterpret_cast<void*>(0)
        );
    glBindVertexArray(0);

    //Set clear colour to a dark green-blueish colour
    glClearColor(.2f, .3f, .3f, 1.f);

    //Use the shader we loaded as our shader program
    objectShader.use();

    //Set up light source and object colours
    glm::vec3 lightSourcePosition {2.f, 2.f, 2.f};
    glm::vec3 lightColor {1.f, 1.f, 1.f};
    glm::vec3 objectColor {1.f, 0.5f, 0.2f};
    GLfloat ambientStrength {.1f};
    GLfloat specularStrength {.8f};
    GLint specularShine {32};
    glUniform3f(lightPositionUniform, lightSourcePosition.x, lightSourcePosition.y, lightSourcePosition.z);
    glUniform3f(lightColorUniform, lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(objectColorUniform, objectColor.r, objectColor.g, objectColor.b);
    glUniform1f(ambientStrengthUniform, ambientStrength);
    glUniform1f(specularStrengthUniform, specularStrength);
    glUniform1i(specularShineUniform, specularShine);

    // Set texture unit for each sampler (in the fragment
    // shader)
    objectShader.setInt("texture1", 0);
    objectShader.setInt("texture2", 1);

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
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE,
            glm::value_ptr(projectionTransform)
        );
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE,
            glm::value_ptr(viewTransform)
        );
        glUniform3f(lightPositionUniform, lightSourcePosition.x, lightSourcePosition.y, lightSourcePosition.z);
        glUniform3f(eyePositionUniform, cameraPosition.x, cameraPosition.y, cameraPosition.z);
        for(glm::vec3 position : cubePositions) {
            // The Model matrix transforms a single object's vertices
            // to its location, orientation, shear, and size, in the 
            // world space
            glm::mat4 model {glm::translate(glm::mat4(1.f), position)};
            glm::mat4 normal { glm::transpose(glm::inverse(model)) };
            glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(normalUniform, 1, GL_FALSE, glm::value_ptr(normal));
            //Start drawing
            glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        //Draw light source
        lightSourceShader.use();
        glUniformMatrix4fv(
            glGetUniformLocation(lightSourceShader.getProgramID(), "projection")
            , 1, GL_FALSE, 
            glm::value_ptr(projectionTransform)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(lightSourceShader.getProgramID(), "view")
            , 1, GL_FALSE, 
            glm::value_ptr(viewTransform)
        );
        glm::mat4 model {glm::mat4(1.f)};
        model = glm::translate(model, lightSourcePosition);
        model = glm::scale(model, glm::vec3(.3f));
        glUniformMatrix4fv(
            glGetUniformLocation(lightSourceShader.getProgramID(), "model"),
            1, GL_FALSE, glm::value_ptr(model)
        );
        //Start drawing
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
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(objectShader.getProgramID());

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
