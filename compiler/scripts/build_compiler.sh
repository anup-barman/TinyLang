#!/bin/bash
set -e

# Go to compiler directory
cd "$(dirname "$0")/.."

mkdir -p build
cd build
if command -v cmake &> /dev/null; then
    cmake ..
    make
else
    echo "CMake not found. Falling back to direct g++ compilation..."
    mkdir -p ../build
    g++ -std=c++20 -O2 ../src/*.cpp -o tinylang-compiler
fi

if [ -f "tinylang-compiler" ]; then
    echo "Compiler built successfully at compiler/build/tinylang-compiler"
else
    echo "Build failed."
    exit 1
fi
