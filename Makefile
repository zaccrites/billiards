
# http://stackoverflow.com/questions/7004702/how-can-i-create-a-makefile-for-c-projects-with-src-obj-and-bin-subdirectories

OS:=$(shell uname)

CXX:=g++
LINKER:=g++

# http://stackoverflow.com/questions/399850/best-compiler-warning-level-for-c-c-compilers
BASE_CXXFLAGS:=-std=c++14 -pedantic -Werror \
	-Wall -Wextra -Weffc++ -Wshadow \
	-Wcast-qual -Wold-style-cast -Wfloat-equal \
	-isystem include
BASE_LDFLAGS:=

BASE_OBJ_DIR:=obj

# To build for debug, invoke with "make DEBUG=1"
DEBUG ?= 0
ifeq ($(DEBUG), 0)
	BASE_CXXFLAGS += -O2 -DNDEBUG
else
	ifeq ($(OS), Darwin)
	  BASE_CXXFLAGS += -g3 -O0 -DDEBUG
    else
	  BASE_CXXFLAGS += -ggdb3 -Og -DDEBUG
    endif
endif

CXXFLAGS=$(BASE_CXXFLAGS)
LDFLAGS:=$(BASE_LDFLAGS) -lSDL2

# Darwin = Mac OSX
ifeq ($(OS), Darwin)
  LDFLAGS += -Wl,-framework,Cocoa
endif


SRC_DIR:=src
OBJ_DIR:=$(BASE_OBJ_DIR)/src
BIN_DIR:=bin
EXECUTABLE=$(BIN_DIR)/billiards

SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)


$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(LINKER) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

# http://stackoverflow.com/a/2501673
DEPS=$(OBJECTS:%.o=%.d)
-include $(DEPS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -MMD -MF $(patsubst %.o,%.d,$@) -o $@



$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)


.PHONY: all
all: $(EXECUTABLE) $(TEST_EXECUTABLE)


.PHONY: clean
clean:
	rm -rf $(BASE_OBJ_DIR) $(BIN_DIR)

