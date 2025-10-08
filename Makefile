CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP
DEBUG_CXXFLAGS = $(CXXFLAGS) -g -O0

BIN_NAME = bin

# Directories
SRC_DIR = src
BUILD_DIR = build
DEBUG_DIR = debug
INCLUDE_DIR = include

TARGET = $(BUILD_DIR)/$(BIN_NAME)
DEBUG_TARGET = $(DEBUG_DIR)/$(BIN_NAME)

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files go in build/ with same basename
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_DIR)/%.o,$(SRCS))

# Dependency files
DEPS = $(OBJS:.o=.d)
DEBUG_DEPS = $(DEBUG_OBJS:.o=.d)

# Default rule
all: build

build: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp to .o in build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: $(DEBUG_TARGET)

# Link object files into the final executable
$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CXX) $(DEBUG_CXXFLAGS) -o $@ $^

# Compile .cpp to .o in build directory
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(DEBUG_DIR)
	$(CXX) $(DEBUG_CXXFLAGS) -c $< -o $@

# Include dependency files
-include $(DEPS)
-include $(DEBUG_DEPS)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR) $(DEBUG_DIR)