#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include "shader.hpp"
#include "texture.hpp"
#include "utility.hpp"

bool init(SDL_Window*& window, SDL_GLContext& context);
void close(SDL_GLContext& context);
void processInput(SDL_Event* event);

int main(int argc, char* argv[]) {
    SDL_Window* window {};
    SDL_GLContext context {};

    // Initialize SDL context
    if(!init(window, context)) {
        close(context);
        return 1;
    }

    //Load shader program
    Shader shader {"shaders/vertex.vs", "shaders/fragment.fs"};
    GLint vertexAttrib { glGetAttribLocation(shader.getProgramID(), "position") };
    GLint colorAttrib { glGetAttribLocation(shader.getProgramID(), "color") };
    GLint textureCoordAttrib{ glGetAttribLocation(shader.getProgramID(), "textureCoord") };

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

    // Set up a polygon to draw; here, a rectangle
    // composed of two adjacent triangles
    float vertices[] {
        -.5f, .5f, // top left
            1.f, 0.f, 0.f, //(red)
            0.f, 1.f, // [sample] texture top left

        .5f, .5f, // top right
            0.f, 1.f, 0.f, //(green)
            1.f, 1.f, // texture top right

        .5f, -.5f, // bottom  right
            0.f, 0.f, 1.f, //(blue)
            1.f, 0.f, // texture bottom right
        
        -.5f, -.5f, // bottom left
            1.f, 1.f, 1.f, //(white)
            0.f, 0.f //texture bottom left
    };

    // Set up element buffer
    GLuint elements[] {
        0, 1, 2, // topright triangle
        0, 2, 3, // bottomleft triangle
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
            2, // Number of values for a "single" input element
            GL_FLOAT, //The format of each component
            GL_FALSE, //Normalize? in case not floating point
            //Format of the attribute array, in terms of
            // |.....|VERTEX_0|.......||.....|VERTEX_1|.......|
            // |-----| --> Offset
            //       |-----------------------| --> Stride
            //       |--------| --> Single vertex element (size of GL_FLOAT)
            7*sizeof(float), // Stride: number of bytes between each position, with 0 indicating there's no offset to the next element
            reinterpret_cast<void*>(0) // Offset: offset of the first element relative to the start of the array
        );
        //Define the format of each vertex color in above buffer
        glVertexAttribPointer(
            colorAttrib, 
            3, GL_FLOAT, GL_FALSE,
            7*sizeof(float), // Stride
            reinterpret_cast<void*>(2*sizeof(float)) // Offset
        );
        glVertexAttribPointer(
            textureCoordAttrib,
            2, GL_FLOAT, GL_FALSE,
            7*sizeof(float), // Stride
            reinterpret_cast<void*>(5*sizeof(float)) // Offset
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

    //Set texture coordinate multiplier for texture 2 (so that it
    // is sampled multiple times)
    shader.setFloat("texture2CoordMultiplier", 1.f);

    //Enable OpenGL blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Main event loop
    SDL_Event event;
    bool wireframeMode { false };

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

            //Enable/disable wireframe mode
            else if(event.type == SDL_KEYUP) {
                switch(event.key.keysym.sym) {
                    case SDLK_9:
                        wireframeMode = !wireframeMode;
                        if(wireframeMode)
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        else 
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    break;
                }
            }
        }
        //Clear colour buffer
        glClear(GL_COLOR_BUFFER_BIT);

        //Start drawing
        glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //Update screen
        SDL_GL_SwapWindow(window);
    }

    glDisable(GL_BLEND);

    // de-allocate resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader.getProgramID());

    close(context);
    return 0;
}

void processInput(SDL_Event* event) {}

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
    window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600,  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    context = SDL_GL_CreateContext(window);
    if(!window || !context) return false;

    //Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    //Set up viewport
    glViewport(0, 0, 800, 600);

    return true;
}

void close(SDL_GLContext& context){
    //Kill the OpenGL context before quitting
    SDL_GL_DeleteContext(context);

    // Then die
    SDL_Quit();
}
