#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cmath>

#include <SDL2/SDL.h>
#include <GL/glew.h>

bool init(SDL_Window*& window, SDL_GLContext& context);
void close(SDL_GLContext& context);

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

    //Retrieve reference to fragment shader uniform attr
    GLint uniColor {glGetUniformLocation(shaderProgram, "triangleColor")};
    glUniform3f(uniColor, 1.f, 0.f, 0.f); // turn our triangle red

    //Set up a polygon to draw; here, a triangle
    float vertices[] {
        0.0f, 0.5f, // top
            1.f, 0.f, 0.f, //(red)
        0.5f, -0.5f, // bottom right
            0.f, 1.f, 0.f, //(green)
        -0.5f, -0.5f, // bottom left
            0.f, 0.f, 1.f //(blue)
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
        GLuint elements[] {
            0, 1, 2
        };
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
            //                |--------------| --> Stride
            //       |--------| --> Single vertex element (size of GL_FLOAT)
            5*sizeof(float), // Stride: number of bytes between each position, with 0 indicating there's no offset to the next element
            0 // Offset: offset of the first element relative to the start of the array
        );
        //Define the format of each vertex color in above buffer
        glVertexAttribPointer(
            colorAttrib, 
            3, GL_FLOAT, GL_FALSE,
            5*sizeof(float), // Stride
            reinterpret_cast<void*>(2*sizeof(float)) //Offset
        );
    glBindVertexArray(0);

    //Main event loop
    SDL_Event windowEvent;
    while(true) {
        if(SDL_PollEvent(&windowEvent)) {
            if(windowEvent.type == SDL_QUIT) break;
            else if(
                windowEvent.type == SDL_KEYUP &&
                windowEvent.key.keysym.sym == SDLK_ESCAPE
            ) break;
        }
        //Clear colour buffer
        glClear(GL_COLOR_BUFFER_BIT);

        //Start drawing
        glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        //Stop drawing
        glBindVertexArray(0);

        //Update screen
        SDL_GL_SwapWindow(window);
    }

    close(context);
    return 0;
}

bool init(SDL_Window*& window, SDL_GLContext& context) {
    //Initialize SDL subsystems
    SDL_Init(SDL_INIT_VIDEO);

    //Specify that we want a forward compatible OpenGL 3.2 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // creating a stencil buffer

    // Create an OpenGL window with SDL
    window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600,  SDL_WINDOW_OPENGL);
    // Create an openGL context
    context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

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

    //Use this shader as our shader program
    glUseProgram(shaderProgram);

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
