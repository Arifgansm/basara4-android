#!/usr/bin/env bash
# Basara 4 Android - Build Script
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
NDK_PATH="${ANDROID_NDK:-}"
TARGET="all"

while [[ $# -gt 0 ]]; do
    case $1 in
        --target) TARGET="$2"; shift 2 ;;
        --ndk) NDK_PATH="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [[ -z "$NDK_PATH" ]]; then
    echo "ERROR: ANDROID_NDK not set"
    exit 1
fi

if [[ ! -f "$NDK_PATH/build/cmake/android.toolchain.cmake" ]]; then
    echo "ERROR: Invalid NDK path"
    exit 1
fi

mkdir -p "$BUILD_DIR"

build_target() {
    local target=$1
    local build_subdir="${BUILD_DIR}/${target}"
    echo "Building $target..."
    mkdir -p "$build_subdir"
    cd "$build_subdir"
    cmake -DCMAKE_TOOLCHAIN_FILE="${NDK_PATH}/build/cmake/android.toolchain.cmake" \
          -DANDROID_ABI=arm64-v8a \
          -DANDROID_PLATFORM=android-29 \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo \
          -DCMAKE_INSTALL_PREFIX="${SCRIPT_DIR}/launcher/src/main/jniLibs" \
          "${SCRIPT_DIR}" \
          -DBUILD_CPU=ON \
          -DBUILD_RUNTIME=ON \
          -DBUILD_GPU=ON
    cmake --build . --target "$target" -- -j$(nproc)
    cmake --install .
}

case "$TARGET" in
    all|cpu|basara4cpu) build_target basara4cpu ;;
    runtime|basara4runtime) build_target basara4runtime ;;
    gpu|basara4gpu) build_target basara4gpu ;;
    *) echo "Unknown target: $TARGET"; exit 1 ;;
esac

echo "Build complete. Libraries in ${SCRIPT_DIR}/launcher/src/main/jniLibs/"