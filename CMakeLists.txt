# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称
project(OGLDEV_Tutorials LANGUAGES CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建输出路径 (可选，方便管理)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# --- 查找依赖项 ---
# 使用 pkg-config 帮助查找库
find_package(PkgConfig REQUIRED)

# OpenGL (通常由驱动提供，但需要头文件)
find_package(OpenGL REQUIRED)

# GLEW (OpenGL Extension Wrangler Library)
find_package(GLEW REQUIRED)
# 从 vcxproj 中提取的定义
add_definitions(-D_USE_MATH_DEFINES)
if(WIN32)
    # GLEW 在 Windows 上可能是静态库，需要定义 GLEW_STATIC
    # 检查 vcxproj 确认，如果 .lib 被链接，通常是静态
    add_definitions(-DGLEW_STATIC)
endif()


# GLFW (窗口和输入库)
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-glfw
# GLFW 的 CMake 配置文件通常能正确处理链接
find_package(glfw3 REQUIRED)

# GLM (OpenGL Mathematics) - 通常是头文件库
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-glm
# find_package 通常能找到头文件路径
find_package(glm REQUIRED)

# Assimp (Open Asset Import Library)
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-assimp
find_package(assimp REQUIRED)

# FreeType (字体渲染库)
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-freetype
find_package(Freetype REQUIRED)

# ImageMagick (图像处理库) - 可选，如果用到再取消注释
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-imagemagick
# find_package(ImageMagick) # ImageMagick 的查找可能需要额外配置

# MeshOptimizer
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-meshoptimizer
# 假设它提供了 CMake 配置文件
find_package(MeshOptimizer REQUIRED CONFIG)

# FreeGLUT
find_package(GLUT REQUIRED) # For tutorials using freeglut directly

# Vulkan (如果教程需要)
# 需要确保 MSYS2 中安装了 vulkan-headers 和 vulkan-loader
find_package(Vulkan REQUIRED)

# Fontconfig (通常与 FreeType 一起使用)
# 需要确保 MSYS2 中安装的是 mingw-w64-x86_64-fontconfig
find_package(Fontconfig REQUIRED)

# --- 设置全局包含目录 ---
# 全局包含目录现在由子项目（ogldev, imgui_lib）通过 target_include_directories PUBLIC 提供
# 因此不再需要在根 CMakeLists.txt 中设置全局 include_directories
# include_directories(
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/include # 旧路径，不再需要
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui         # 旧路径，不再需要
# )

# 添加 MSYS2/MINGW64 的库目录 (pkg-config 通常会处理，但如果遇到链接问题可以取消注释)
# link_directories($ENV{MINGW_PREFIX}/lib)

# --- 添加子目录 ---
# 添加核心库和 ImGui
add_subdirectory(src/common)
add_subdirectory(src/imgui)

# 添加第一个教程
add_subdirectory(src/Tutorial01)

# 暂时注释掉动态添加其他教程的部分
# file(GLOB TUTORIAL_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/Tutorial*")
# # ... (其他项目目录) ...
# foreach(PROJECT_DIR ${ALL_PROJECT_DIRS})
#     if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}/CMakeLists.txt")
#         message(STATUS "Adding subdirectory: ${PROJECT_DIR}")
#         add_subdirectory(${PROJECT_DIR})
#     else()
#         message(WARNING "Skipping directory (not found or no CMakeLists.txt): ${PROJECT_DIR}")
#     endif()
# endforeach()

message(STATUS "Root CMakeLists.txt updated. Added common, imgui, and Tutorial01 subdirectories.")