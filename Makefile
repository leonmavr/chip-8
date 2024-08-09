CC = g++
TARGET = demo
SRC_DIR = src
INC_DIR = include
CFLAGS = -std=c++17 -I$(INC_DIR) -Wall
LDFLAGS = 
DEMO_SRC = demo.cpp
TEST_SRC = test/tests.cpp
SRC = $(wildcard $(SRC_DIR)/*.cpp) $(DEMO_SRC)

# Compare cmd arguments; if `test`, extend it to handle unit tests
ifeq ($(MAKECMDGOALS), test)
    CFLAGS += -DRUN_UNIT_TESTS
    SRC = $(wildcard $(SRC_DIR)/*.cpp) $(TEST_SRC)
endif

OBJECTS = $(SRC:%.cpp=%.o)
RM = rm -rf

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

.PHONY: clean

clean:
	$(RM) $(TARGET)
	$(RM) $(SRC_DIR)/*.o
	$(RM) demo.o
	$(RM) test/*.o

