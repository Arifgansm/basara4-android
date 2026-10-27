# Basara 4 Android - FEXCore CPU Layer CMake Configuration

# FEXCore submodule path (adjust as needed)
set(FEXCORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/FEX" CACHE PATH "FEXCore source path")

# Check if FEXCore is available
if(EXISTS "${FEXCORE_PATH}/CMakeLists.txt")
    message(STATUS "FEXCore found at ${FEXCORE_PATH}")
    
    # Add FEXCore as subdirectory
    add_subdirectory("${FEXCORE_PATH}" "${CMAKE_BINARY_DIR}/fexcore" EXCLUDE_FROM_ALL)
else()
    message(WARNING "FEXCore not found at ${FEXCORE_PATH}")
    message(WARNING "Please run: git submodule update --init --recursive third_party/FEX")
endif()

# Basara 4 CPU library (FEXCore backend)
add_library(basara4cpu SHARED
    guest_cpu_fex.cpp
    host_call_arm64.S
)

# Link against FEXCore
if(TARGET FEXCore)
    target_link_libraries(basara4cpu PRIVATE FEXCore)
endif()

# Android-specific settings
target_compile_definitions(basara4cpu PRIVATE
    GUEST_CPU_FEX
    __aarch64__
)

target_compile_options(basara4cpu PRIVATE
    -O2
    -fPIC
    -fno-omit-frame-pointer
)

target_include_directories(basara4cpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Install to APK
install(TARGETS basara4cpu LIBRARY DESTINATION lib/arm64-v8a)
