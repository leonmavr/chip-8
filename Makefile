###################################################
# definitions                                     #
###################################################
CC = g++

# for project target
EXEC = demo 
SRC_DIR = src
INCL_DIR = include
CFLAGS = -std=c++17 -g -I$(INCL_DIR) -Wall
LDFLAGS = 
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) \
    $(wildcard $(INCL_DIR)/*.hpp) \
	demo.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) $(EXEC)
	$(RM) $(SRC_DIR)/*.o
