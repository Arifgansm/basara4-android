#!/usr/bin/env bash
# Gradle Wrapper Script for Android projects
# This is a minimal wrapper - in reality you'd use the official gradle wrapper

# Check if gradle is available
if command -v gradle >/dev/null 2>&1; then
    exec gradle "$@"
elif [ -f "./gradlew" ]; then
    exec ./gradlew "$@"
else
    echo "ERROR: Gradle not found. Please install gradle or use Android Studio."
    echo "To install in Termux: pkg install gradle"
    exit 1
fi