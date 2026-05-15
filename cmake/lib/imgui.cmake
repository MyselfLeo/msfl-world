# message("----- IMGUI -----")

include_directories(lib/imgui)
include_directories(lib/imgui/backends)

add_library(imgui STATIC lib/imgui/imgui.cpp lib/imgui/imgui.h lib/imgui/imgui_demo.cpp lib/imgui/imgui_draw.cpp lib/imgui/imgui_internal.h lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/imstb_rectpack.h lib/imgui/imstb_textedit.h lib/imgui/imstb_truetype.h)
target_include_directories(imgui PUBLIC lib/imgui/)

add_library(imgui_impl_glfw lib/imgui/backends/imgui_impl_glfw.h lib/imgui/backends/imgui_impl_glfw.cpp)
target_include_directories(imgui_impl_glfw PUBLIC lib/imgui/)

add_library(imgui_impl_opengl3 lib/imgui/backends/imgui_impl_opengl3.h lib/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_impl_opengl3 PUBLIC lib/imgui/)