#OBJS specifies which files to compile as part of the project
OBJS = src/main.c \
src/array.c \
src/camera.c \
src/clip.c \
src/display.c \
src/draw_triangle_pikuma.c \
src/draw_triangle_torb.c \
src/func.c \
src/light.c \
src/matrix.c \
src/mesh.c \
src/misc.c \
src/render_font/software_bitmapfont.c \
src/texture.c \
src/upng.c \
src/vecmath.c \
src/vertex_shading.c
#CC specifies which compiler we're using
#CC = g++ or gcc or clang or clang++
CC ?= gcc

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -I/mingw64/include/

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -L/mingw64/lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
#COMPILER_FLAGS = -w -Wl,-subsystem,windows -DNO_STDIO_REDIRECT
COMPILER_FLAGS = -pipe -Wall -Wextra -Wno-double-promotion -Wno-sign-compare -O0 #-fsanitize-undefined-trap-on-error -fsanitize=undefined -fsanitize=bounds #-fsanitize=memory
COMPILER_FLAGS := $(COMPILER_FLAGS) -std=c99 -m64
#COMPILER_FLAGS := $(COMPILER_FLAGS) -std=c++0x -fpermissive # compile as C++

COMPILER_FLAGS_RELEASE = -pipe -Wall -Wextra -Wdouble-promotion -Wno-sign-compare -O3 -DNDEBUG
COMPILER_FLAGS_RELEASE := $(COMPILER_FLAGS_RELEASE) -std=c99 -O2
#COMPILER_FLAGS_RELEASE := $(COMPILER_FLAGS) -std=c++0x -fpermissive -Os #-ffast-math -Os # compile as C++

#LINKER_FLAGS specifies the libraries we're linking against
#LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lm
LINKER_FLAGS = -lSDL2main -lSDL2 -lm

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = software_painter.exe

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

release : $(OBJS)
		$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_RELEASE) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	./software_painter assets/f22.obj assets/f22.png
clean:
	rm -f ./a
