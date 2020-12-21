CC = g++

EXEC = chip8
SRC_DIR = src
INCL_DIR = include
CFLAGS = -std=c++14 -g -I$(INCL_DIR)
LDFLAGS = -lSDL2
SOURCES = $(SRC_DIR)/chip8.cpp $(SRC_DIR)/demo.cpp $(SRC_DIR)/display.cpp $(SRC_DIR)/keyboard.cpp 
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf


all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 

$(BLD_DIR)%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(EXEC)
