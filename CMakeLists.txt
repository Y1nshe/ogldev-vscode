# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称
project(OGLDEV_Tutorials LANGUAGES CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置全局构建输出路径 (可执行文件到 build/bin, 库到 build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- 在 build/bin 目录创建指向源 Content 目录的链接 ---
if(EXISTS "${CMAKE_SOURCE_DIR}/Content")
    # 定义链接所在的目录 (build/bin)
    set(LINK_PARENT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(LINK_NAME "Content") # 链接名称
    set(TARGET_DIR "${CMAKE_SOURCE_DIR}/Content") # 链接指向的目标（源目录）
    set(LINK_PATH "${LINK_PARENT_DIR}/${LINK_NAME}") # 链接的完整路径 (build/bin/Content)

    # 确保链接所在的目录存在
    file(MAKE_DIRECTORY "${LINK_PARENT_DIR}")

    # 检查链接是否已存在且有效
    if(NOT EXISTS "${LINK_PATH}")
        message(STATUS "[Content Link] Creating link/junction in ${LINK_PARENT_DIR}: ${LINK_NAME} -> ${TARGET_DIR}")
        if(WIN32)
            # Windows: mklink /J <Link> <Target>
            # 计算相对于链接所在目录的目标路径
            file(RELATIVE_PATH TARGET_REL_PATH "${LINK_PARENT_DIR}" "${TARGET_DIR}")
            string(REPLACE "/" "\\" TARGET_REL_PATH_WIN "${TARGET_REL_PATH}")
            message(STATUS "[Content Junction] Attempting to create junction point: ${LINK_NAME} -> ${TARGET_REL_PATH_WIN} in ${LINK_PARENT_DIR}")
            execute_process(COMMAND cmd /c mklink /J "${LINK_NAME}" "${TARGET_REL_PATH_WIN}"
                            WORKING_DIRECTORY "${LINK_PARENT_DIR}" # 在 build/bin 目录下执行 mklink
                            RESULT_VARIABLE MKDIR_RESULT
                            OUTPUT_VARIABLE MKDIR_OUTPUT
                            ERROR_VARIABLE MKDIR_ERROR)
        else()
            # Unix: ln -s <Target> <Link>
            # 计算相对于链接所在目录的目标路径
            file(RELATIVE_PATH TARGET_REL_PATH "${LINK_PARENT_DIR}" "${TARGET_DIR}")
            execute_process(COMMAND ln -s "${TARGET_REL_PATH}" "${LINK_NAME}"
                            WORKING_DIRECTORY "${LINK_PARENT_DIR}" # 在 build/bin 目录下执行 ln
                            RESULT_VARIABLE MKDIR_RESULT
                            OUTPUT_VARIABLE MKDIR_OUTPUT
                            ERROR_VARIABLE MKDIR_ERROR)
        endif()

        if(NOT MKDIR_RESULT EQUAL 0)
            message(WARNING "[Content Link/Junction] Failed to create link/junction '${LINK_PATH}'. Error code: ${MKDIR_RESULT}. Output: ${MKDIR_OUTPUT}. Error: ${MKDIR_ERROR}")
        else()
             message(STATUS "[Content Link/Junction] Successfully created link/junction in ${LINK_PARENT_DIR}.")
        endif()
    else()
         message(STATUS "[Content Link/Junction] Link '${LINK_PATH}' already exists. Skipping creation.")
    endif()
else()
    message(WARNING "[Content Link] Source directory ${CMAKE_SOURCE_DIR}/Content not found. Skipping link creation.")
endif()
# --- /链接逻辑结束 ---

# --- 在 build/bin/common 目录创建指向源 Shaders 目录的链接 ---
if(EXISTS "${CMAKE_SOURCE_DIR}/src/common/Shaders")
    set(LINK_PARENT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/common")
    set(LINK_NAME "Shaders") # 链接名称
    set(TARGET_DIR "${CMAKE_SOURCE_DIR}/src/common/Shaders") # 目标为源 Shaders 目录
    set(LINK_PATH "${LINK_PARENT_DIR}/${LINK_NAME}") # 链接的完整路径 (build/bin/common/Shaders)

    # 确保链接所在的目录存在
    file(MAKE_DIRECTORY "${LINK_PARENT_DIR}")

    # 检查链接是否已存在且有效
    if(NOT EXISTS "${LINK_PATH}")
        message(STATUS "[Shaders Link] Creating link/junction in ${LINK_PARENT_DIR}: ${LINK_NAME} -> ${TARGET_DIR}")
        if(WIN32)
            # Windows: mklink /J <Link> <Target>
            file(RELATIVE_PATH TARGET_REL_PATH "${LINK_PARENT_DIR}" "${TARGET_DIR}")
            string(REPLACE "/" "\\" TARGET_REL_PATH_WIN "${TARGET_REL_PATH}") # Ensure backslashes are escaped for CMake string
            message(STATUS "[Shaders Junction] Attempting to create junction point: ${LINK_NAME} -> ${TARGET_REL_PATH_WIN} in ${LINK_PARENT_DIR}")
            execute_process(COMMAND cmd /c mklink /J "${LINK_NAME}" "${TARGET_REL_PATH_WIN}"
                            WORKING_DIRECTORY "${LINK_PARENT_DIR}"
                            RESULT_VARIABLE MKDIR_RESULT
                            OUTPUT_VARIABLE MKDIR_OUTPUT
                            ERROR_VARIABLE MKDIR_ERROR)
        else()
            # Unix: ln -s <Target> <Link>
            file(RELATIVE_PATH TARGET_REL_PATH "${LINK_PARENT_DIR}" "${TARGET_DIR}")
            execute_process(COMMAND ln -s "${TARGET_REL_PATH}" "${LINK_NAME}"
                            WORKING_DIRECTORY "${LINK_PARENT_DIR}"
                            RESULT_VARIABLE MKDIR_RESULT
                            OUTPUT_VARIABLE MKDIR_OUTPUT
                            ERROR_VARIABLE MKDIR_ERROR)
        endif()

        if(NOT MKDIR_RESULT EQUAL 0)
            message(WARNING "[Shaders Link/Junction] Failed to create link/junction '${LINK_PATH}'. Error code: ${MKDIR_RESULT}. Output: ${MKDIR_OUTPUT}. Error: ${MKDIR_ERROR}")
        else()
             message(STATUS "[Shaders Link/Junction] Successfully created link/junction in ${LINK_PARENT_DIR}.")
        endif()
    else()
         message(STATUS "[Shaders Link/Junction] Link '${LINK_PATH}' already exists. Skipping creation.")
    endif()
else()
    message(WARNING "[Shaders Link] Source directory ${CMAKE_SOURCE_DIR}/src/common/Shaders not found. Skipping link creation.")
endif()
# --- /链接逻辑结束 ---

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
# Revert to basic find_package, hoping it defines GLFW_LIBRARIES
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
add_subdirectory(src/DemoLITION)

# 在添加 freetype-gl 子目录之前设置选项，禁用文档、示例和测试构建
set(freetype-gl_BUILD_APIDOC OFF CACHE BOOL "Disable freetype-gl documentation build" FORCE)
set(freetype-gl_BUILD_DEMOS OFF CACHE BOOL "Disable freetype-gl demo build" FORCE)
set(freetype-gl_BUILD_TESTS OFF CACHE BOOL "Disable freetype-gl test build" FORCE)
add_subdirectory(src/freetype-gl)

add_subdirectory(src/Experiment01)

add_subdirectory(src/Tutorials/Tutorial01)
add_subdirectory(src/Tutorials/Tutorial02)
add_subdirectory(src/Tutorials/Tutorial03)
add_subdirectory(src/Tutorials/Tutorial04)
add_subdirectory(src/Tutorials/Tutorial05)
add_subdirectory(src/Tutorials/Tutorial06)
add_subdirectory(src/Tutorials/Tutorial07)
add_subdirectory(src/Tutorials/Tutorial08)
add_subdirectory(src/Tutorials/Tutorial09)
add_subdirectory(src/Tutorials/Tutorial10)
add_subdirectory(src/Tutorials/Tutorial11)
add_subdirectory(src/Tutorials/Tutorial12)
add_subdirectory(src/Tutorials/Tutorial13)
add_subdirectory(src/Tutorials/Tutorial14)
add_subdirectory(src/Tutorials/Tutorial15)
add_subdirectory(src/Tutorials/Tutorial16)
add_subdirectory(src/Tutorials/Tutorial17)
add_subdirectory(src/Tutorials/Tutorial18)
add_subdirectory(src/Tutorials/Tutorial19)
add_subdirectory(src/Tutorials/Tutorial20)
add_subdirectory(src/Tutorials/Tutorial21)
add_subdirectory(src/Tutorials/Tutorial22)
add_subdirectory(src/Tutorials/Tutorial23)
add_subdirectory(src/Tutorials/Tutorial24)
add_subdirectory(src/Tutorials/Tutorial25)
add_subdirectory(src/Tutorials/Tutorial25_assimp_sandbox)
add_subdirectory(src/Tutorials/Tutorial26)
add_subdirectory(src/Tutorials/Tutorial27)
add_subdirectory(src/Tutorials/Tutorial28)
add_subdirectory(src/Tutorials/Tutorial28_Particles)
add_subdirectory(src/Tutorials/Tutorial29)
add_subdirectory(src/Tutorials/Tutorial30)
add_subdirectory(src/Tutorials/Tutorial31)
add_subdirectory(src/Tutorials/Tutorial32)
add_subdirectory(src/Tutorials/Tutorial33)
add_subdirectory(src/Tutorials/Tutorial34)
add_subdirectory(src/Tutorials/Tutorial35)
add_subdirectory(src/Tutorials/Tutorial36)
add_subdirectory(src/Tutorials/Tutorial37)
add_subdirectory(src/Tutorials/Tutorial38)
add_subdirectory(src/Tutorials/Tutorial39)
add_subdirectory(src/Tutorials/Tutorial40)
add_subdirectory(src/Tutorials/Tutorial41)
add_subdirectory(src/Tutorials/Tutorial42)
add_subdirectory(src/Tutorials/Tutorial43)
add_subdirectory(src/Tutorials/Tutorial44)
add_subdirectory(src/Tutorials/Tutorial45)
add_subdirectory(src/Tutorials/Tutorial45_demo1)
add_subdirectory(src/Tutorials/Tutorial46_FreetypeGL)
add_subdirectory(src/Tutorials/Tutorial47_2DTess)
add_subdirectory(src/Tutorials/Tutorial48_MeshOptimizer)
add_subdirectory(src/Tutorials/Tutorial49_WireframeOnMesh)
add_subdirectory(src/Tutorials/Tutorial50_DSA)
add_subdirectory(src/Tutorials/Tutorial51_QuadTess)
add_subdirectory(src/Tutorials/Tutorial52_NormalMapping)
add_subdirectory(src/Tutorials/Tutorial54_EndlessGrid)
add_subdirectory(src/Tutorials/Tutorial55_GLMCamera)
add_subdirectory(src/Tutorials/Tutorial56_Cubemaps)
add_subdirectory(src/Tutorials/Tutorial57_ComputeShaderParticles)
add_subdirectory(src/Tutorials/Tutorial58_DrawIndirect)
add_subdirectory(src/Tutorials/Tutorial59_BindlessTextures)
add_subdirectory(src/Tutorials/Tutorial60_RefRefract)

# 暂时注释掉动态添加其他教程的部分
# file(GLOB TUTORIAL_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/Tutorials/Tutorial*")
# # ... (其他项目目录) ...
# foreach(PROJECT_DIR ${ALL_PROJECT_DIRS})
#     if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}/CMakeLists.txt")
#         message(STATUS "Adding subdirectory: ${PROJECT_DIR}")
#         add_subdirectory(${PROJECT_DIR})
#     else()
#         message(WARNING "Skipping directory (not found or no CMakeLists.txt): ${PROJECT_DIR}")
#     endif()
# endforeach()

message(STATUS "Root CMakeLists.txt updated. Added common, imgui, DemoLITION, freetype-gl, Experiment01 and Tutorial subdirectories.")