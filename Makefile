SRCS := main.cpp shader.cpp texture.cpp utility.cpp flycamera.cpp light.cpp mesh.cpp model.cpp

CC := g++

INCLUDE_PATHS := -ID:\MyDev\MinGW64\Include

LIBRARY_PATHS := -LD:\MyDev\MinGW64\Lib

LINKER_FLAGS := -lmingw32 -lSDL2main -lSDL2 -lOpenGL32 -lglew32 -lSDL2_image -lSDL2_image.dll -lassimp.dll

COMPILER_FLAGS_DBG := -Wall

DEBUG_OPTS := -fdiagnostics-color=always -g

COMPILER_FLAGS := $(COMPILER_FLAGS_DBG) -Wl,-subsystem,windows

OBJS := my_opengl_demo

all : $(SRCS)
	$(CC) $(SRCS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJS)

debug : $(SRCS)
	$(CC) $(SRCS) $(INCLUDE_PATHS)  $(LIBRARY_PATHS) $(COMPILER_FLAGS_DBG) $(LINKER_FLAGS) $(DEBUG_OPTS) -o $(OBJS)
