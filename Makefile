#OBJS specifies which files to compile as part of the project
OBJS = src/main.c src/func.c src/display.c src/vector.c src/mesh.c src/array.c src/matrix.c src/light.c

#CC specifies which compiler we're using
#CC = g++ or gcc
CC = clang

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -IC:\dev\SDL2-2.0.14\x86_64-w64-mingw32\include\

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:\dev\SDL2-2.0.14\x86_64-w64-mingw32\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
#COMPILER_FLAGS = -w -Wl,-subsystem,windows
COMPILER_FLAGS = -pipe -Wall -Wextra -Wdouble-promotion -Wno-sign-compare -DNO_STDIO_REDIRECT -g -o2 -fsanitize-undefined-trap-on-error# -fsanitize=undefined -fsanitize=bounds #-fsanitize=memory
COMPILER_FLAGS := $(COMPILER_FLAGS) -std=c99
#COMPILER_FLAGS := $(COMPILER_FLAGS) -std=c++0x -fpermissive # compile as C++

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lm

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = a

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	./a parameter 1 2 3
clean:
	rm ./a
