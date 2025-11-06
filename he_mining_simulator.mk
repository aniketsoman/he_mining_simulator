CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
TARGET = he_mining_simulator
SRC_DIR = src
BUILD_DIR = mkbuild

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the program (make run ARGS="--trucks 10 --unload-sites 10")
run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET) $(ARGS)

.PHONY: all clean run
