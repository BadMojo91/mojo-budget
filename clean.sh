#!/usr/bin/env bash
set -e

ROOT_DIR="$(dirname "$0")"

usage() {
  echo "Usage: $0 [linux|windows|all]"
  echo "  linux    - Clean Linux build directory (build/linux)"
  echo "  windows  - Clean Windows build directory (build/win64)"
  echo "  all      - Clean all build directories (default)"
  exit 0
}

clean_linux() {
  echo "Cleaning Linux build..."
  rm -rf "$ROOT_DIR/build/linux"
}

clean_windows() {
  echo "Cleaning Windows build..."
  rm -rf "$ROOT_DIR/build/win64"
}

TARGET="${1:-all}"

case "$TARGET" in
  linux)   clean_linux ;;
  windows) clean_windows ;;
  all)     clean_linux; clean_windows ;;
  -h|--help) usage ;;
  *) echo "Unknown target: $TARGET"; usage ;;
esac

echo "Clean complete."
