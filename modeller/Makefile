# Makefile for 3D Primitives OpenGL Program
# 
# Usage:
#   make          - Compiles the program
#   make run      - Compiles and runs the program
#   make clean    - Removes compiled files
#   make debug    - Compiles with debug symbols

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra

# Libraries to link
# -lGL: OpenGL library
# -lGLEW: OpenGL Extension Wrangler
# -lglfw: GLFW window management
# -lm: Math library
LIBS = -lGL -lGLEW -lglfw -lm

# Source files
SRC = modeller.cpp

# Output executable name
TARGET = modeller

# Default target - compiles the program
all: $(TARGET)

# Compile the program
$(TARGET): $(SRC)
	@echo "Compiling $(SRC)..."
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)
	@echo "Build successful! Run with: ./$(TARGET)"

# Compile and run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Debug build (with debug symbols and no optimization)
debug: CXXFLAGS += -g -O0
debug: clean $(TARGET)
	@echo "Debug build complete!"

# Optimized build
release: CXXFLAGS += -O3
release: clean $(TARGET)
	@echo "Release build complete!"

# Clean up compiled files
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)
	@echo "Clean complete!"

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install -y build-essential libglew-dev libglfw3-dev libglm-dev

# Install dependencies (Fedora/RHEL)
install-deps-fedora:
	@echo "Installing dependencies for Fedora/RHEL..."
	sudo dnf install -y gcc-c++ glew-devel glfw-devel glm-devel

# Install dependencies (Arch Linux)
install-deps-arch:
	@echo "Installing dependencies for Arch Linux..."
	sudo pacman -S --needed base-devel glew glfw-x11 glm

# Help message
help:
	@echo "Available targets:"
	@echo "  make              - Compile the program"
	@echo "  make run          - Compile and run the program"
	@echo "  make debug        - Compile with debug symbols"
	@echo "  make release      - Compile with optimizations"
	@echo "  make clean        - Remove compiled files"
	@echo "  make install-deps - Install dependencies (Ubuntu/Debian)"
	@echo "  make help         - Show this help message"

# Phony targets (not actual files)
.PHONY: all run debug release clean install-deps install-deps-fedora install-deps-arch help
