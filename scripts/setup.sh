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
    local BIN_DIR="$BUILD_DIR/bin"

    echo "[INFO] Searching for executable in $BIN_DIR..."

    # Find the first executable file in the bin directory
    # -maxdepth 1 prevents searching subdirectories
    # -type f finds files
    # -executable finds executable files
    # -print prints the path
    # -quit exits find immediately after the first match (since you expect only one)
    local EXECUTABLE_PATH=$(find "$BIN_DIR" -maxdepth 1 -type f -executable -print -quit)

    # Check if an executable was found
    if [ -z "$EXECUTABLE_PATH" ]; then
        echo "[ERROR] No executable found in $BIN_DIR."
        echo "[ERROR] Ensure the project has been built successfully (e.g., './setup.sh build')."
        exit 1
    fi

    # Run the found executable
    echo "[INFO] Running $(basename "$EXECUTABLE_PATH")..."
    "$EXECUTABLE_PATH"
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

