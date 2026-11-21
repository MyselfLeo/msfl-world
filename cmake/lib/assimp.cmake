# No debug postfix
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)

# No viewer
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)

# Build Assimp as static lib
set(BUILD_SHARED_LIBS OFF)

# Build Zlib
set(ASSIMP_BUILD_ZLIB ON)

# Using Assimp as a submodule
set(ASSIMP_INSTALL OFF)

# Disable tests
set(ASSIMP_BUILD_TESTS OFF)

# Disable samples
set(ASSIMP_BUILD_SAMPLES OFF)

# Disable supplementary tools
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

# Disable export functionalities
set(ASSIMP_NO_EXPORT ON)

# Disable Warnings as errors for Assimp
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

message("----- ASSIMP -----")

if (MSFLW_USE_ASSIMP)
    target_compile_definitions(msfl_world PUBLIC -DMSFLW_ENABLE_ASSIMP=1)
    message(STATUS "Including ASSIMP")

    add_subdirectory(lib/assimp)
    target_include_directories(msfl_world PUBLIC lib/assimp/include)
    target_link_libraries(msfl_world PUBLIC assimp)
else()
    target_compile_definitions(msfl_world PUBLIC -DMSFLW_ENABLE_ASSIMP=0)
    message(STATUS "ASSIMP explicitly disabled")
endif()