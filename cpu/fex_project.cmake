# FEXCore project configuration for Basara 4 Android
# Source this file after setting up FEXCore paths

if(NOT DEFINED FEXCORE_PATH)
    message(FATAL_ERROR "FEXCORE_PATH not set. Set it to your FEXCore source directory.")
endif()

# FEXCore build options for Android
set(FEXOPTS
    -DFEX_ENABLE_LTO=ON
    -DFEX_BUILD_DOC=OFF
    -DFEX_DISABLE_LOADER=ON
    -DBUILD_TESTS=OFF
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
)
