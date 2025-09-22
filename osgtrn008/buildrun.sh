#!/bin/bash

# Stop on any error
set -e

# Go to the script's directory
cd "$(dirname "$0")"

# Create build folder if it doesn't exist
mkdir -p build
cd build

# Clean build folder
rm -rf *

# Run CMake to configure project
cmake ..

# Build the project
make

# Run the executable (assumes your target is named osgtrn001)
./osgtrn008