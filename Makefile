OBJS = src/display_test.cpp src/display.cpp
CC = g++

COMPILER_FLAGS = -Wall -Isrc

LINKER_FLAGS = -lSDL2

OBJ_NAME = display_test

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
