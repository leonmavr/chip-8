CC = g++

EXEC = chip8
SRC_DIR = src
INCL_DIR = include
CFLAGS = -std=c++14 -g -I$(INCL_DIR)
LDFLAGS = -lSDL2
SOURCES = $(SRC_DIR)/chip8.cpp $(SRC_DIR)/demo.cpp $(SRC_DIR)/display.cpp $(SRC_DIR)/keyboard.cpp $(SRC_DIR)/toot.cpp $(SRC_DIR)/logger.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf


all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 

# TODO: needs a make test which defines MAX_ITER to be ~600 so that it exits and I can check the screendump

$(BLD_DIR)%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(EXEC)
