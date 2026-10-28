# Basara 4 Android - FEXCore CPU Layer CMake
# Links against pre-built FEXCore shared library.

# ── FEXCore pre-built library ──────────────────────────────────
if(DEFINED ENV{FEXCORE_PATH} OR FEXCORE_PATH)
    set(FEXCORE_PATH $ENV{FEXCORE_PATH} CACHE PATH "Path to FEXCore installation")
endif()

find_library(FEXCORE_LIB 
    NAMES FEXCore
    PATHS ${FEXCORE_PATH}/lib /usr/local/lib
    NO_DEFAULT_PATH
)

if(NOT FEXCORE_LIB)
    message(FATAL_ERROR "FEXCore library not found. Set FEXCORE_PATH or install FEXCore.")
endif()

message(STATUS "Found FEXCore: ${FEXCORE_LIB}")

# ── Basara 4 CPU library (with real FEXCore JIT) ───────────────
add_library(basara4cpu SHARED
    guest_cpu_fex.cpp
    host_call_arm64.S
)

target_include_directories(basara4cpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${FEXCORE_PATH}/include  # FEXCore headers
)

target_link_libraries(basara4cpu PRIVATE
    ${FEXCORE_LIB}
    log
    android
)

target_compile_definitions(basara4cpu PRIVATE
    GUEST_CPU_FEX
    __aarch64__
)

target_compile_options(basara4cpu PRIVATE 
    -O2 -fPIC -fno-omit-frame-pointer
)

install(TARGETS basara4cpu LIBRARY DESTINATION lib/arm64-v8a)
install(FILES ${FEXCORE_LIB} DESTINATION lib/arm64-v8a)
