# Build GLM as static library
set(GLM_STATIC_LIBRARY_ENABLE ON)

# Don't build install
set(GLM_BUILD_INSTALL OFF)

# Enable C++ 20
# set(GLM_ENABLE_CXX_20 ON)

message("----- GLM -----")

add_subdirectory(lib/glm)
target_include_directories(msfl_world PUBLIC lib/glm)
target_link_libraries(msfl_world PUBLIC glm)