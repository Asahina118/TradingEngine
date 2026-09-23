#!/bin/bash
set -e
BUILD_DIR="build"
if [ "$1" == "clean" ]; then
    echo "Cleaning old build files..."
    rm -rf $BUILD_DIR
fi

mkdir -p $BUILD_DIR
cmake -B $BUILD_DIR -G Ninja
cmake --build $BUILD_DIR

echo "build complete"

./$BUILD_DIR/exchange_run