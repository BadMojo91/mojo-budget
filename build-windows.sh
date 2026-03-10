#!/usr/bin/env bash
set -e

BUILD_DIR="$(dirname "$0")/build/win64"
SOURCE_DIR="$(dirname "$0")"
TOOLCHAIN="$SOURCE_DIR/toolchain-mingw64.cmake"

mkdir -p "$BUILD_DIR"

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DBUILD_NCURSES=OFF

cmake --build "$BUILD_DIR"

echo "Windows build complete: $BUILD_DIR"
