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

#include "shader.hpp"
#include "texture.hpp"
#include "utility.hpp"

//Initialize camera variables
glm::vec3 gCameraPos { 0.f, 0.f, 3.f };
glm::vec3 gCameraFront { 0.f, 0.f, -1.f };
glm::vec3 gCameraUp  { 0.f, 1.f,  0.f };
glm::vec3 gCameraVelocity {0.f, 0.f, 0.f};
float gCameraYaw {glm::radians(-90.f)}; // make camera point down -Z axis by default
float gCameraPitch {0.f};
float gFOV{45.f};

const float CameraTopSpeed {2.5f}; // in coordinates/second
const float CameraLookSensitivity {0.004f};

bool gWireframeMode { false };

float gDeltaTime {0.f};
bool gMouseWarped {true};

SDL_Window* gWindow{nullptr};
const int WindowHeight {600};
const int WindowWidth {800};

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
    Shader shader {"shaders/vertex.vs", "shaders/fragment.fs"};
    GLint vertexAttrib { glGetAttribLocation(shader.getProgramID(), "position") };
    GLint colorAttrib { glGetAttribLocation(shader.getProgramID(), "color") };
    GLint textureCoordAttrib{ glGetAttribLocation(shader.getProgramID(), "textureCoord") };
    //Model, view, projection matrices
    GLint modelUniform { glGetUniformLocation(shader.getProgramID(), "model")};
    GLint viewUniform { glGetUniformLocation(shader.getProgramID(), "view")};
    GLint projectionUniform { glGetUniformLocation(shader.getProgramID(), "projection")};

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

    //Set up a polygon to draw; here, a triangle
    float vertices[] {
        -.5f, .5f, .5f, // topleft front
            1.f, 0.f, 0.f, //(red)
            0.f, 1.f, // [sample] texture top left
        .5f, .5f, 0.5f, // topright front
            1.f, 1.f, 0.f, // (yellow)
            1.f, 1.f, // texture top right
        .5f, -.5f, 0.5f, // bottom right front
            0.f, 1.f, 0.f, //(green)
            1.f, 0.f, // texture bottom right
        -.5f, -.5f, 0.5f, // bottom  left front
            0.f, 0.f, 1.f, //(blue)
            0.f, 0.f, // texture bottom left
        
        -.5f, -.5f, -.5f, // bottom left back
            0.f, 0.f, 0.f,
            0.f, 1.f,
        .5f, -.5f, -.5f, // bottom right back
            0.f, 0.f, 0.f,
            1.f, 1.f,
        .5f, .5f, -.5f, // top right back
            0.f, 0.f, 0.f,
            1.f, 0.f,
        -.5f, .5f, -.5f, // top left back
            0.f, 0.f, 0.f,
            0.f, 0.f
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
        //Specify which buffer to use for vertices, elements
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
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
            8*sizeof(float), // Stride: number of bytes between each position, with 0 indicating there's no offset to the next element
            reinterpret_cast<void*>(0) // Offset: offset of the first element relative to the start of the array
        );
        //Define the format of each vertex color in above buffer
        glVertexAttribPointer(
            colorAttrib, 
            3, GL_FLOAT, GL_FALSE,
            8*sizeof(float), // Stride
            reinterpret_cast<void*>(3*sizeof(float)) // Offset
        );
        glVertexAttribPointer(
            textureCoordAttrib,
            2, GL_FLOAT, GL_FALSE,
            8*sizeof(float), // Stride
            reinterpret_cast<void*>(6*sizeof(float)) // Offset
        );
    glBindVertexArray(0);

    //Set clear colour to a dark green-blueish colour
    glClearColor(.2f, .3f, .3f, 1.f);

    //Use the shader we loaded as our shader program
    shader.use();

    // Set texture unit for each sampler (in the fragment
    // shader)
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    // Render 5 instances of the cube at the following
    // positions
    glm::vec3 cubePositions[] {
        glm::vec3(0.f, 0.f, -2.f),
        glm::vec3(2.f, 5.f, -15.f),
        glm::vec3(-1.5f, -2.2f, -12.3f),
        glm::vec3(5.f,-0.3f, -28.f),
        glm::vec3(-3.f, -7.2f, -14.7f)
    };

    // The projection matrix will transform our vertices from world space to clip 
    // space, more on that here: https://jsantell.com/3d-projection/
    glm::mat4 projection{glm::mat4(1.f)};
    // make this projection matrix out of parametrs FOV and e(aspect ratio, essentially width/height)
    projection = glm::perspective(glm::radians(45.f), 800.f/600.f, .1f, 100.f);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));

    // The View matrix, the inverse of the position of the camera,
    // transforms vertices such that they are located relative 
    // to the camera's position, with the camera at (0,0,0)
    glm::mat4 view { glm::mat4(1.f) };

    uint64_t lastFrame {SDL_GetTicks64()}; // time of last frame

    //Main event loop
    SDL_Event event;

    SDL_CaptureMouse(SDL_TRUE);
    SDL_ShowCursor(SDL_FALSE);
    while(true) {
        //Check SDL event queue for any events, process them
        if(SDL_PollEvent(&event)) {
            //Handle exit events
            if(event.type == SDL_QUIT) break;
            else if(
                event.type == SDL_KEYUP &&
                event.key.keysym.sym == SDLK_ESCAPE
            ) break;

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
        //Update time related variables
        uint64_t currentFrame {SDL_GetTicks64()};
        gDeltaTime = static_cast<float>(currentFrame - lastFrame)/1000.f;
        lastFrame = currentFrame;

        //Update camera orientation
        glm::vec3 direction;
        direction.x = cos(gCameraPitch)*cos(gCameraYaw);
        direction.z = cos(gCameraPitch)*sin(gCameraYaw);
        direction.y = sin(gCameraPitch);

        //Update camera position
        gCameraPos += gCameraVelocity.z * direction * gDeltaTime;
        gCameraPos += gCameraVelocity.x * (glm::normalize(glm::cross(direction, gCameraUp))) * gDeltaTime;

        view = glm::lookAt(
            gCameraPos, // camera position
            gCameraPos + direction, // camera target
            gCameraUp // arbitrary up vector
        );
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));

        projection = glm::perspective(glm::radians(gFOV), 800.f/600.f, .1f, 100.f);
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));

        //Clear colour and depth buffers before each render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(glm::vec3 position : cubePositions) {
            // The Model matrix transforms a single object's vertices
            // to its location, orientation, shear, and size, in the 
            // world space
            glm::mat4 model {glm::mat4(1.f)};
            model = glm::translate(model, position);
            model = glm::rotate(model, 
                glm::radians(static_cast<float>(SDL_GetTicks())/10.f),
                position
            );
            glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));

            //Start drawing
            glBindVertexArray(vao);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        //Update screen
        SDL_GL_SwapWindow(gWindow);
    }
    SDL_ShowCursor(SDL_TRUE);
    SDL_CaptureMouse(SDL_FALSE);

    // de-allocate resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader.getProgramID());

    close(context);
    return 0;
}

void processInput(SDL_Event* event) {

    //Handle mouse movement
    if(event->type == SDL_MOUSEMOTION) {
        if(gMouseWarped){ 
            gMouseWarped = false;
            return;
        }
        float dX { static_cast<float>(event->motion.xrel) };
        float dY { -static_cast<float>(event->motion.yrel) };
        gCameraYaw += CameraLookSensitivity * dX;
        gCameraPitch += CameraLookSensitivity * dY;
        gMouseWarped = true;
        SDL_WarpMouseInWindow(gWindow, WindowWidth/2, WindowHeight/2);
    }
    else if(event->type == SDL_MOUSEWHEEL){
        std::cout << "mousewheel event occurred\n";
        gFOV += static_cast<float>(event->wheel.y);
        if(gFOV > 90.f) gFOV = 90.f;
        else if(gFOV < 40.f) gFOV = 40.f;
        std::cout << "FOV: " << gFOV << '\n';
    }

    //Handle keydown
    else if(event->type == SDL_KEYDOWN){
        switch(event->key.keysym.sym) {
            case SDLK_w:
                gCameraVelocity.z = CameraTopSpeed;
            break;
            case SDLK_a:
                gCameraVelocity.x = -CameraTopSpeed;
            break;
            case SDLK_s:
                gCameraVelocity.z = -CameraTopSpeed;
            break;
            case SDLK_d:
                gCameraVelocity.x = CameraTopSpeed;
            break;
        }
    }

    // Handle keyup
    else if(event->type == SDL_KEYUP) {
        switch(event->key.keysym.sym) {
            //enable/disable wireframe mode
            case SDLK_9:
                gWireframeMode = !gWireframeMode;
            break;

            // handle camera movements
            case SDLK_w:
                gCameraVelocity.z = 0.f;
            break;
            case SDLK_a:
                gCameraVelocity.x = 0.f;
            break;
            case SDLK_s:
                gCameraVelocity.z = 0.f;
            break;
            case SDLK_d:
                gCameraVelocity.x = 0.f;
            break;
        }
    }
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
    window = SDL_CreateWindow("OpenGL", 100, 100, WindowWidth, WindowHeight,  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
