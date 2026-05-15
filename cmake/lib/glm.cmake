# Build GLM as static library
set(GLM_STATIC_LIBRARY_ENABLE ON)

# Don't build install
set(GLM_BUILD_INSTALL OFF)

# Enable C++ 20
# set(GLM_ENABLE_CXX_20 ON)

message("----- GLM -----")

add_subdirectory(lib/glm)
include_directories(lib/glm)