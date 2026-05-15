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

message("----- ASSIMP -----")

add_subdirectory(lib/assimp)
include_directories(lib/assimp/include)