###################################################
# definitions                                     #
###################################################
CC = g++

# for project target
EXEC = demo 
SRC_DIR = src
INCL_DIR = include
CFLAGS = -std=c++17 -g -I$(INCL_DIR) -Wall
LDFLAGS = -lSDL2
SOURCES = $(wildcard $(SRC_DIR)/*cpp) \
	demo.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf

# for unit test target
TEST_DIR = tests
TEST_EXEC = $(TEST_DIR)/test
TEST_SOURCES = $(wildcard $(SRC_DIR)/*cpp)
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

.PHONY: quickclean clean

quickclean:
	$(RM) $(OBJECTS)
	$(RM) $(EXEC)
	$(RM) $(TEST_OBJECTS)
	$(RM) $(TEST_EXEC)
	$(RM) $(TEST_DIR)/tests.to

clean: quickclean
	$(RM) $(TEST_DIR)/*.to

###################################################
# unit tests                                      #
###################################################

%.to: %.cpp
	$(CC) $(TEST_CFLAGS) -c $^ -o $@

# test case file
tests.to: $(TEST_DIR)/tests.cpp
	$(CC) $(TEST_DIR)/tests.cpp $(TEST_CFLAGS) -o $(TEST_DIR)/tests.to

# test library (Catch2)
catch.to: $(TEST_DIR)/catch.cpp
	$(CC) -c $(TEST_DIR)/catch.cpp $(TEST_CFLAGS) -o $(TEST_DIR)/catch.to

# compile and run unit tests
test: $(TEST_DIR)/tests.to $(TEST_DIR)/catch.to $(TEST_OBJECTS)
	$(CC) $(TEST_DIR)/tests.to $(TEST_DIR)/catch.to $(TEST_OBJECTS) -o $(TEST_EXEC) $(LDFLAGS) 
	./$(TEST_EXEC)
