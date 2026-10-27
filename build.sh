#!/bin/bash
# Basara 4 Android - Quick Build Script for Termux
# This script attempts to build a debug APK with available tools

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/launcher"
BUILD_OUTPUT="${PROJECT_DIR}/build/outputs/apk/debug"

echo "=== Basara 4 Android Build Script ==="
echo "Project directory: ${PROJECT_DIR}"
echo ""

# Check prerequisites
echo "Checking prerequisites..."
command -v java >/dev/null || { echo "ERROR: java not found"; exit 1; }
command -v javac >/dev/null || { echo "ERROR: javac not found"; exit 1; }

JAVA_VERSION=$(java -version 2>&1 | head -1)
echo "Java: ${JAVA_VERSION}"

# Check for Gradle
if command -v gradle >/dev/null; then
    GRADLE_BIN="gradle"
    GRADLE_VERSION=$(gradle --version 2>&1 | head -1)
    echo "Gradle: ${GRADLE_VERSION}"
elif [ -f "${SCRIPT_DIR}/gradlew" ]; then
    GRADLE_BIN="./gradlew"
else
    echo "WARNING: Gradle not found. Install with: pkg install gradle"
    echo "WARNING: Or download from https://gradle.org/install/"
    echo "Proceeding with manual build steps..."
fi

# Check for Android SDK
ANDROID_HOME="${ANDROID_HOME:-${HOME}/Android/Sdk}"
if [ ! -d "${ANDROID_HOME}" ]; then
    echo "WARNING: ANDROID_HOME not set or not found"
    echo "Set ANDROID_HOME to your Android SDK path"
    ANDROID_HOME=""
fi

if [ -n "${ANDROID_HOME}" ]; then
    if [ ! -d "${ANDROID_HOME}/platforms" ]; then
        echo "WARNING: No Android platforms found in ${ANDROID_HOME}/platforms"
    else
        echo "Android SDK: ${ANDROID_HOME}"
    fi
fi

# Check for NDK
ANDROID_NDK="${ANDROID_NDK_HOME:-${ANDROID_HOME}/ndk}"
if [ ! -d "${ANDROID_NDK}" ]; then
    echo "WARNING: ANDROID_NDK_HOME not set or not found"
fi

echo ""
echo "=== Build Steps ==="

# Step 1: Compile native code (if NDK available)
if [ -n "${ANDROID_NDK}" ] && [ -d "${ANDROID_NDK}" ]; then
    echo "Step 1: Building native libraries..."
    # TODO: Implement native build
    echo "Native build requires FEXCore and shadPS4 GPU core"
    echo "Skipping native build for now"
else
    echo "Step 1: NDK not available, skipping native build"
fi

# Step 2: Compile Java/Kotlin code
echo "Step 2: Compiling Java/Kotlin sources..."
# TODO: Implement Java/Kotlin compilation

# Step 3: Package resources
echo "Step 3: Packaging resources..."
# TODO: Implement resource packaging

# Step 4: Generate APK
echo "Step 4: Generating APK..."
# TODO: Implement APK generation

echo ""
echo "=== Build Complete ==="
echo "To build a complete APK, use Android Studio:"
echo "  1. Open this project in Android Studio"
echo "  2. Configure local.properties with your SDK/NDK paths"
echo "  3. Build > Build Bundle(s) / APK(s) > Build APK(s)"
echo ""
echo "Or using command line with Gradle:"
echo "  ./gradlew assembleDebug"
echo ""
echo "Note: This is a WIP project. The APK will only show the launcher screen."
echo "Full game functionality requires FEXCore and shadPS4 GPU core integration."