# Basara 4 Android - Top-level CMake Configuration
cmake_minimum_required(VERSION 3.22)
project(basara4_android LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ANDROID_NDK ${ANDROID_NDK})

option(BUILD_CPU "Build CPU layer (FEXCore)" ON)
option(BUILD_RUNTIME "Build runtime layer" ON)
option(BUILD_GPU "Build GPU layer (shadPS4)" ON)
option(BUILD_LAUNCHER "Build Android launcher" ON)

# FEXCore path - can be set via -DFEXCORE_PATH=... or env var
if(NOT DEFINED FEXCORE_PATH)
    set(FEXCORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FEX" CACHE PATH "FEXCore source path")
endif()

if(ANDROID)
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
    set(CMAKE_ANDROID_NDK ${ANDROID_NDK})
    set(CMAKE_ANDROID_PLATFORM 29)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden -std=c++20")
    
    find_library(LOG_LIB log)
    find_library(ANDROID_LIB android)
    find_library(EGL_LIB EGL)
    find_library(GLES_LIB GLESv3)
    find_library(VULKAN_LIB vulkan)
    
    set(ANDROID_LIBS ${LOG_LIB} ${ANDROID_LIB} ${EGL_LIB} ${GLES_LIB} ${VULKAN_LIB})
endif()

if(BUILD_CPU)
    # Include FEXCore if available
    if(EXISTS "${FEXCORE_PATH}/CMakeLists.txt")
        message(STATUS "FEXCore found at ${FEXCORE_PATH}")
        add_subdirectory("${FEXCORE_PATH}" "${CMAKE_BINARY_DIR}/fexcore" EXCLUDE_FROM_ALL)
    else()
        message(WARNING "FEXCore not found at ${FEXCORE_PATH}")
        message(WARNING "Build will fail without FEXCore. Download from https://github.com/FEX-Emu/FEX")
    endif()
    add_subdirectory(cpu)
endif()

if(BUILD_RUNTIME)
    add_subdirectory(src)
endif()

if(BUILD_GPU)
    add_subdirectory(gpu)
endif()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/arm64-v8a/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/jniLibs/arm64-v8a
        FILES_MATCHING PATTERN "*.so")