OBJS = src/demo.cpp src/display.cpp src/chip8.cpp
CC = g++

COMPILER_FLAGS = -Wall -Isrc

LINKER_FLAGS = -lSDL2

OBJ_NAME = demo

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
