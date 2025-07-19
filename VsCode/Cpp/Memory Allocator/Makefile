# Compiler settings
CXX      := clang++
CXXFLAGS := -std=gnu++17 -g -Wall -Iinclude

# Directories
SRCDIR   := src
INCDIR   := include
BUILDDIR := build
BINDIR   := bin

# Find all .cpp files in src/, then map to .o in build/
SRC      := $(wildcard $(SRCDIR)/*.cpp)
OBJ      := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))

# Name of the executable (placed in bin/)
TARGET   := myProgram

# Default target: make the bin/ directory and then build the executable
all: $(BINDIR)/$(TARGET)

# Link step: ensure bin/ exists, then link all objects
$(BINDIR)/$(TARGET): $(OBJ) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile step: ensure build/ exists, then compile each .cpp → .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build/ and bin/ directories on demand
$(BUILDDIR) $(BINDIR):
	mkdir -p $@

# Remove all build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)/* $(BINDIR)/*

