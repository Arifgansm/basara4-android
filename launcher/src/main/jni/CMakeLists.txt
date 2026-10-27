cmake_minimum_required(VERSION 3.22)
project(basara4jni LANGUAGES C CXX)

# This CMake is for the JNI layer only
# The actual native libraries (basara4cpu, basara4runtime, basara4gpu)
# are prebuilt and placed in jniLibs/

# JNI wrapper library
add_library(basara4jni SHARED
    native_lib.cpp
)

# Link against Android libraries and our prebuilt libraries
find_library(LOG_LIB log)
find_library(ANDROID_LIB android)

target_link_libraries(basara4jni
    ${LOG_LIB}
    ${ANDROID_LIB}
    basara4runtime
    basara4cpu
)

# Include directories
target_include_directories(basara4jni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cpu
)

# Compiler options
target_compile_options(basara4jni PRIVATE
    -std=c++20
    -fPIC
    -fvisibility=hidden
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# This library will be copied to jniLibs by the build system
set_target_properties(basara4jni PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../jniLibs/${ANDROID_ABI}
)