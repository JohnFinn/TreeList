# Default recipe - install, configure, and build
default: install configure build

# Install Conan dependencies
install:
    conan install . --output-folder=build --build=missing

# Configure CMake with Conan toolchain
configure:
    cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Build the project
build:
    cmake --build build

# Run tests
test: default
    ./build/tree_list_test

# Clean build artifacts
clean:
    rm -rf build

# Full rebuild from scratch
rebuild: clean default
