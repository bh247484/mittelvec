CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP
BIN_NAME = bin

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TARGET = $(BUILD_DIR)/$(BIN_NAME)

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Object files go in build/ with same basename
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
# Dependency files
DEPS = $(OBJS:.o=.d)

# Default rule
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp to .o in build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Make sure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Include dependency files
-include $(DEPS)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)