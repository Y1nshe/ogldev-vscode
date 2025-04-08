# 定义 imgui 静态库
add_library(imgui_lib STATIC)

# 添加 ImGui 源文件 (路径相对于项目根目录 CMAKE_SOURCE_DIR)
# 注意：我们现在直接引用根目录下的 Common/3rdparty/ImGui/GLFW 文件夹
target_sources(imgui_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui.cpp
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp
)

# 添加包含目录 (对外暴露原始 ImGui 文件夹)
target_include_directories(imgui_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/Common/3rdparty/ImGui/GLFW
)

# 链接 ImGui 实现所需的库 (保持不变)
target_link_libraries(imgui_lib PUBLIC
    OpenGL::GL
    glfw # 使用 glfw3 可能更明确
)

message(STATUS "CMakeLists.txt for imgui library updated to use sources/includes from root Common/3rdparty/ImGui/GLFW.")