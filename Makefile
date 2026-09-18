CC = g++
TARGET = play 
SRC_DIR = src
INC_DIR = include
CFLAGS = -std=c++17 -I$(INC_DIR) -Wall
DEMO_SRC = demo.cpp
TEST_SRC = test/tests.cpp

# Optional ALSA sound support. Build without it via: make SOUND=0
SOUND ?= 1
ifeq ($(SOUND),1)
    CFLAGS += -DCHIP8_ENABLE_SOUND
    LDFLAGS += -lasound
    SOUND_SRC = $(SRC_DIR)/beeper.cpp
endif

# beeper.cpp is added only when sound is enabled, so a SOUND=0 build needs no
# ALSA headers or libraries at all.
SRC = $(filter-out $(SRC_DIR)/beeper.cpp,$(wildcard $(SRC_DIR)/*.cpp)) $(SOUND_SRC) $(DEMO_SRC)

# Compare cmd arguments; if `test`, extend it to handle unit tests
ifeq ($(MAKECMDGOALS), test)
    CFLAGS += -DRUN_UNIT_TESTS
    SRC = $(filter-out $(SRC_DIR)/beeper.cpp,$(wildcard $(SRC_DIR)/*.cpp)) $(SOUND_SRC) $(TEST_SRC)
    TARGET = test/test
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

