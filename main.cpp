#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cmath>

#include <SDL2/SDL.h>
#include <GL/glew.h>

bool init(SDL_Window*& window, SDL_GLContext& context);
void close(SDL_GLContext& context);
void processInput(SDL_Event* event);

bool loadShaderProgram(GLuint& shaderProgram, GLint& vertexAttrib, GLint& colorAttrib);

int main(int argc, char* argv[]) {
    SDL_Window* window {};
    SDL_GLContext context {};

    // Initialize SDL context
    if(!init(window, context)) {
        close(context);
        return 1;
    }

    //Load shader program
    GLuint shaderProgram {};
    GLint vertexAttrib {};
    GLint colorAttrib {};
    if(!loadShaderProgram(shaderProgram, vertexAttrib, colorAttrib)) {
        close(context);
        return 2;
    }

    //Set up polygon(s) to draw; here, 2 triangles
    float vertices[] {
        //triangle 1
        -.25f, .5f, // top
            1.f, //(white)
        0.f, -.5f, // bottom right
            0.f, //(black)
        -.5f, -.5f, // bottom  left
            0.5f, //(medium gray)

        //Triangle 2
        .25f, .5f, // top
            0.f, // (black)
        .5f, -.5f, // bottom right
            .5f, // (medium gray)
        0.f, -.5f, //bottom left
            1.f // (white)
    };

    // Set up element buffer
    GLuint elements[] {
        0, 1, 2, //  triangle
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
            3*sizeof(float), // Stride: number of bytes between each position, with 0 indicating there's no offset to the next element
            reinterpret_cast<void*>(0) // Offset: offset of the first element relative to the start of the array
        );
        //Define the format of each vertex color in above buffer
        glVertexAttribPointer(
            colorAttrib, 
            1, GL_FLOAT, GL_FALSE,
            3*sizeof(float), // Stride
            reinterpret_cast<void*>(2*sizeof(float)) //Offset
        );
    glBindVertexArray(0);

    //Set clear colour to a dark green-blueish colour
    glClearColor(.2f, .3f, .3f, 1.f);

    //Use the shader we loaded as our shader program
    glUseProgram(shaderProgram);

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

        //Draw
        glBindVertexArray(vao);
            // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //Update screen
        SDL_GL_SwapWindow(window);
    }

    // de-allocate resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);

    close(context);
    return 0;
}

void processInput(SDL_Event* event) {}

bool init(SDL_Window*& window, SDL_GLContext& context) {
    //Initialize SDL subsystems
    SDL_Init(SDL_INIT_VIDEO);

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

bool loadShaderProgram(GLuint& shaderProgram, GLint& vertexAttrib, GLint& colorAttrib) {
    //Load vertex shader source from file
    std::ifstream vShaderFile{"shaders/vertex.glvs"};
    if(!vShaderFile) {
        std::cerr << "Could not read vertex shader!" << std::endl;
        return false;
    }
    std::string vShaderStr {};
    for(std::string inputStr {}; std::getline(vShaderFile, inputStr);) {
        vShaderStr += inputStr;
        vShaderStr.push_back('\n');
    }
    vShaderFile.close();
    const char* vShaderSource {vShaderStr.c_str()};

    // Compile vertex shader
    GLuint vertexShader {glCreateShader(GL_VERTEX_SHADER)};
    GLint vShaderCompileStatus {};
    glShaderSource(vertexShader, 1, &vShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompileStatus);
    if(vShaderCompileStatus != GL_TRUE) {
        char glslCompileError[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, glslCompileError);
        std::cout << glslCompileError << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    //Load fragment shader source
    std::ifstream fShaderFile{"shaders/fragment.glfs"};
    if(!fShaderFile) {
        std::cerr << "Could not read fragment shader!" << std::endl;
        return false;
    }
    std::string fShaderStr {};
    for(std::string inputStr {}; std::getline(fShaderFile, inputStr);) {
        fShaderStr += inputStr;
        fShaderStr.push_back('\n');
    }
    fShaderFile.close();
    const char* fShaderSource {fShaderStr.c_str()};

    //Compile fragment shader
    GLuint fragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
    glShaderSource(fragmentShader, 1, &fShaderSource, NULL);
    GLint fShaderCompileStatus {};
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompileStatus);
    if(fShaderCompileStatus != GL_TRUE) {
        char glslCompileError[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, glslCompileError);
        std::cout << glslCompileError << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    //Link the shaders together into a single shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // (happens by default anyway, so commented out) specify 
    // that framebuffer 0 is where outColor is written to
    // glBindFragDataLocation (shaderProgram, 0,"outColor");
    GLint programLinkStatus {};
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinkStatus);
    if(programLinkStatus != GL_TRUE) {
        char glslLinkError[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, glslLinkError);
        std::cout << glslLinkError << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        return false;
    }

    // Delete shader objects; we no longer require
    // them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Retrieve pointers to shader attributes
    vertexAttrib = glGetAttribLocation(shaderProgram, "position");
    colorAttrib = glGetAttribLocation(shaderProgram, "color");

    return true;
}

void close(SDL_GLContext& context){


    //Kill the OpenGL context before quitting
    SDL_GL_DeleteContext(context);

    // Then die
    SDL_Quit();
}
