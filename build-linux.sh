#!/usr/bin/env bash
set -e

BUILD_DIR="$(dirname "$0")/build/linux"
SOURCE_DIR="$(dirname "$0")"

mkdir -p "$BUILD_DIR"

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja
cmake --build "$BUILD_DIR"

echo "Linux build complete: $BUILD_DIR"
