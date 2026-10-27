# Basara 4 Android - Runtime Layer CMake
cmake_minimum_required(VERSION 3.22)

# Runtime library
add_library(basara4runtime SHARED
    runtime.c
    runtime_cpu.c
    runtime_offline.c
    runtime_audio.c
    runtime_pad.c
    probe.c
)

# Platform-specific sources
if(ANDROID)
    target_sources(basara4runtime PRIVATE
        # Android-specific implementations
    )
    target_link_libraries(basara4runtime PRIVATE
        ${ANDROID_LIBS}
        basara4cpu  # FEXCore backend
        libatrac9   # ATRAC9 audio decoder
    )
else()
    target_link_libraries(basara4runtime PRIVATE
        dl pthread m libatrac9
    )
endif()

# Include directories
target_include_directories(basara4runtime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/LibAtrac9/C/src
)

# Compile definitions
target_compile_definitions(basara4runtime PRIVATE
    _GNU_SOURCE
    ANDROID
    GUEST_CPU_FEX
)

# Compiler options
target_compile_options(basara4runtime PRIVATE
    -O2
    -fPIC
    -fvisibility=hidden
    -Wall -Wextra
    -Wno-unused-parameter
)

# Link options
target_link_options(basara4runtime PRIVATE
    -Wl,--no-undefined
)

# Install
install(TARGETS basara4runtime LIBRARY DESTINATION lib/arm64-v8a)