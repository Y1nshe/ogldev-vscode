# 定义可执行文件目标
# 注意：源文件现在指向根目录下的 tutorial01 文件夹
add_executable(Tutorial01 ${CMAKE_SOURCE_DIR}/tutorial01/tutorial01.cpp)

# 链接所需库
# 链接我们自己创建的 ogldev 和 imgui_lib 静态库
# CMake 会自动处理这些库的 PUBLIC 依赖项 (如 GLEW, GLFW, GLM, etc.)
target_link_libraries(Tutorial01 PRIVATE
    ogldev
    imgui_lib
    GLUT::GLUT # Link against FreeGLUT
    # 如果这个特定教程需要其他 MINGW 库，可以在这里添加
    # 例如：如果教程直接使用了 Vulkan，可以添加 Vulkan::Vulkan
    # 但通常依赖项应该由 ogldev 库传递过来
)

# 如果需要，可以为这个特定目标添加额外的包含目录或定义
# target_include_directories(Tutorial01 PRIVATE ...)
# target_compile_definitions(Tutorial01 PRIVATE ...)

message(STATUS "CMakeLists.txt for Tutorial01 updated to use source from root tutorial01 folder.")