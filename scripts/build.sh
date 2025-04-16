#!/bin/bash

# Resolve the directory this script is in and move one level up to get the project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Function to build the project using CMake with C99
build() {
    echo "[INFO] Building project..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || exit 1
    cmake -DCMAKE_C_STANDARD=99 "$PROJECT_ROOT"
    make || exit 1
}

# Function to run the main executable
run() {
    EXECUTABLE_NAME=$(basename "$PROJECT_ROOT")
    EXECUTABLE_PATH="$BUILD_DIR/bin/$EXECUTABLE_NAME"

    if [ -f "$EXECUTABLE_PATH" ]; then
        echo "[INFO] Running $EXECUTABLE_NAME..."
        "$EXECUTABLE_PATH"
    else
        echo "[ERROR] Executable not found. Try './dev.sh build' first."
        exit 1
    fi
}

# Function to clean the build directory
clean() {
    echo "[INFO] Cleaning build directory..."
    rm -rf "$BUILD_DIR"
}

# Main logic
case "$1" in
    build)
        build
        ;;
    run)
        run
        ;;
    clean)
        clean
        ;;
    "")
        build && run
        ;;
    *)
        echo "Usage: $0 {build|run|clean}"
        ;;
esac

