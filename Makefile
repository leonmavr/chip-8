###################################################
# definitions                                     #
###################################################
CC = g++

EXEC = chip8
SRC_DIR = src
INCL_DIR = include
CFLAGS = -std=c++17 -g -I$(INCL_DIR) -Wall
LDFLAGS = -lSDL2
SOURCES = $(SRC_DIR)/chip8.cpp $(SRC_DIR)/demo.cpp $(SRC_DIR)/display.cpp $(SRC_DIR)/keyboard.cpp $(SRC_DIR)/toot.cpp $(SRC_DIR)/logger.cpp $(SRC_DIR)/ini_reader.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf

TEST_DIR = tests
TEST_EXEC = $(TEST_DIR)/test
TEST_SOURCES = $(SRC_DIR)/chip8.cpp $(SRC_DIR)/display.cpp $(SRC_DIR)/keyboard.cpp $(SRC_DIR)/toot.cpp $(SRC_DIR)/logger.cpp $(SRC_DIR)/ini_reader.cpp
TEST_OBJECTS = $(TEST_SOURCES:%.cpp=%.to)
TEST_CFLAGS = -std=c++17 -g -I$(INCL_DIR)

###################################################
# project                                         #
###################################################
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 


%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@


.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(EXEC)
	$(RM) $(TEST_OBJECTS)
	$(RM) $(TEST_EXEC)
	$(RM) $(TEST_DIR)/*.to


###################################################
# unit tests                                      #
###################################################

%.to: %.cpp
	$(CC) $(TEST_CFLAGS) -c $^ -o $@

tests.to: $(TEST_DIR)/tests.cpp
	$(CC) $(TEST_DIR)/tests.cpp $(TEST_CFLAGS) -o $(TEST_DIR)/tests.to

catch.to: $(TEST_DIR)/catch.cpp
	$(CC) $(TEST_DIR)/catch.cpp $(TEST_CFLAGS) -o $(TEST_DIR)/catch.to

test: $(TEST_DIR)/tests.to $(TEST_DIR)/catch.to $(TEST_OBJECTS)
	$(CC) $(TEST_DIR)/tests.to $(TEST_DIR)/catch.to $(TEST_OBJECTS) -o $(TEST_EXEC) $(LDFLAGS) 
	./$(TEST_EXEC)
