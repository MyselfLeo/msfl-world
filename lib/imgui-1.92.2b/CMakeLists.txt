add_library(imgui STATIC imgui.cpp imgui.h imgui_demo.cpp imgui_draw.cpp imgui_internal.h imgui_tables.cpp imgui_widgets.cpp imstb_rectpack.h imstb_textedit.h imstb_truetype.h)
target_include_directories(imgui PUBLIC ".")

add_library(imgui_impl_glfw backends/imgui_impl_glfw.h backends/imgui_impl_glfw.cpp)
target_include_directories(imgui_impl_glfw PUBLIC ".")

add_library(imgui_impl_opengl3 backends/imgui_impl_opengl3.h backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_impl_opengl3 PUBLIC ".")