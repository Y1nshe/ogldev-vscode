cmake_minimum_required(VERSION 3.16) # 提高版本要求以使用更现代的 CMake 功能

# 定义教程名称和编号 (方便引用)
set(TUTORIAL_NAME Tutorial58_DrawIndirect)
set(TUTORIAL_NUM 58)

# --- 源文件 ---
# 列出此教程特定的源文件 (相对于此 CMakeLists.txt)
set(TUTORIAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tutorial58.cpp
)

# --- 资源文件 ---
# 列出需要复制到构建目录的资源文件 (相对于此 CMakeLists.txt)
set(RESOURCE_FILES

)

# --- 依赖项列表 (用于检查是否需要 DemoLITION 资源) ---
set(DEPENDENCIES_LIST GLEW::glew
    assimp::assimp
    demolition_lib
    glfw
    meshoptimizer)

# --- 定义可执行目标 ---
add_executable(${TUTORIAL_NAME} ${TUTORIAL_SOURCES})

# --- 新增：设置目标的可执行文件输出目录 ---
# 将此教程的可执行文件放在 bin/<TutorialName> 子目录下
set_target_properties(${TUTORIAL_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TUTORIAL_NAME}")

# --- 设置包含目录 ---
# 确保可以找到此目录中复制过来的头文件
target_include_directories(${TUTORIAL_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# --- 链接库 ---
# 基础库 + 教程特定库
target_link_libraries(${TUTORIAL_NAME} PRIVATE
    ogldev      # 我们的核心库
    imgui_lib   # ImGui 集成库
    ${DEPENDENCIES_LIST} # 动态添加的其他依赖
)

# --- DemoLITION 资源处理 (如果需要) ---
list(FIND DEPENDENCIES_LIST "demolition_lib" _demolition_index)
if(_demolition_index GREATER -1)
    message(STATUS "[${TUTORIAL_NAME}] Depends on demolition_lib, configuring shader directory link.")
    set(DEMOLITION_SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/src/DemoLITION/Shaders/GL") # 源目录
    # 目标链接路径: build/bin/<TutorialName>/Framework/Shaders/GL
    set(DEMOLITION_SHADER_LINK_PARENT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TUTORIAL_NAME}/Framework/Shaders") # 链接所在的父目录
    set(DEMOLITION_SHADER_LINK_NAME "GL") # 链接名
    set(DEMOLITION_SHADER_LINK_PATH "${DEMOLITION_SHADER_LINK_PARENT_DIR}/${DEMOLITION_SHADER_LINK_NAME}") # 完整的链接路径

    # 检查源目录是否存在
    if(EXISTS "${DEMOLITION_SHADER_SRC_DIR}")
        # 确保链接的父目录存在
        file(MAKE_DIRECTORY "${DEMOLITION_SHADER_LINK_PARENT_DIR}")

        # 检查链接是否已存在
        if(NOT EXISTS "${DEMOLITION_SHADER_LINK_PATH}")
            message(STATUS "[${TUTORIAL_NAME}] Creating link/junction for DemoLITION shaders: ${DEMOLITION_SHADER_LINK_PATH} -> ${DEMOLITION_SHADER_SRC_DIR}")

            # 计算从链接父目录到源目录的相对路径
            file(RELATIVE_PATH TARGET_REL_PATH "${DEMOLITION_SHADER_LINK_PARENT_DIR}" "${DEMOLITION_SHADER_SRC_DIR}")

            # 根据操作系统执行不同的链接命令
            if(WIN32)
                # Windows: mklink /J <LinkName> <RelativeTargetPath>
                string(REPLACE "/" "\\" TARGET_REL_PATH_WIN "${TARGET_REL_PATH}")
                message(STATUS "[${TUTORIAL_NAME}] Windows: mklink /J "${DEMOLITION_SHADER_LINK_NAME}" "${TARGET_REL_PATH_WIN}" in ${DEMOLITION_SHADER_LINK_PARENT_DIR}")
                execute_process(COMMAND cmd /c mklink /J "${DEMOLITION_SHADER_LINK_NAME}" "${TARGET_REL_PATH_WIN}"
                                WORKING_DIRECTORY "${DEMOLITION_SHADER_LINK_PARENT_DIR}"
                                RESULT_VARIABLE MKDIR_RESULT
                                OUTPUT_VARIABLE MKDIR_OUTPUT
                                ERROR_VARIABLE MKDIR_ERROR)
            else()
                # Unix: ln -s <RelativeTargetPath> <LinkName>
                 message(STATUS "[${TUTORIAL_NAME}] Unix: ln -s "${TARGET_REL_PATH}" "${DEMOLITION_SHADER_LINK_NAME}" in ${DEMOLITION_SHADER_LINK_PARENT_DIR}")
                execute_process(COMMAND ln -s "${TARGET_REL_PATH}" "${DEMOLITION_SHADER_LINK_NAME}"
                                WORKING_DIRECTORY "${DEMOLITION_SHADER_LINK_PARENT_DIR}"
                                RESULT_VARIABLE MKDIR_RESULT
                                OUTPUT_VARIABLE MKDIR_OUTPUT
                                ERROR_VARIABLE MKDIR_ERROR)
            endif()

            # 检查链接创建是否成功
            if(NOT MKDIR_RESULT EQUAL 0)
                message(WARNING "[${TUTORIAL_NAME}] Failed to create link/junction for DemoLITION shaders at '${DEMOLITION_SHADER_LINK_PATH}'. Error code: ${MKDIR_RESULT}. Output: ${MKDIR_OUTPUT}. Error: ${MKDIR_ERROR}")
            else()
                 message(STATUS "[${TUTORIAL_NAME}] Successfully created link/junction for DemoLITION shaders.")
            endif()
        else()
             message(STATUS "[${TUTORIAL_NAME}] Link/junction for DemoLITION shaders '${DEMOLITION_SHADER_LINK_PATH}' already exists. Skipping creation.")
        endif()
    else()
        message(WARNING "[${TUTORIAL_NAME}] DemoLITION shader source directory not found: ${DEMOLITION_SHADER_SRC_DIR}. Skipping link creation.")
    endif()
else()
     message(STATUS "[${TUTORIAL_NAME}] Does not depend on demolition_lib, skipping DemoLITION shader link creation.")
endif()

# --- 教程特定资源文件处理 ---
if(RESOURCE_FILES)
    # 为资源文件创建一个目标，以便在需要时复制它们
    set(COPIED_RESOURCES "")
    foreach(RESOURCE_REL_PATH ${RESOURCE_FILES})
        # 获取绝对路径以进行复制
        get_filename_component(RESOURCE_ABS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_REL_PATH} ABSOLUTE)
        # 获取文件名以用作目标路径
        get_filename_component(RESOURCE_FILENAME ${RESOURCE_REL_PATH} NAME)
        # 修改：将目标路径设置为可执行文件输出目录下的教程子目录 (build/bin/<TutorialName>)
        set(DESTINATION_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TUTORIAL_NAME}/${RESOURCE_FILENAME}")

        # 添加自定义命令，仅在文件更改时复制
        add_custom_command(
            OUTPUT ${DESTINATION_PATH}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RESOURCE_ABS_PATH} ${DESTINATION_PATH}
            DEPENDS ${RESOURCE_ABS_PATH}  # 依赖于源文件
            COMMENT "Copying ${RESOURCE_FILENAME} for ${TUTORIAL_NAME}"
            VERBATIM
        )
        list(APPEND COPIED_RESOURCES ${DESTINATION_PATH})
    endforeach()

    # 创建一个聚合目标来管理所有资源的复制
    add_custom_target(CopyResources_${TUTORIAL_NAME} ALL DEPENDS ${COPIED_RESOURCES})

    # 使可执行文件依赖于资源复制目标
    add_dependencies(${TUTORIAL_NAME} CopyResources_${TUTORIAL_NAME})

    # 获取资源文件的数量
    list(LENGTH RESOURCE_FILES NUM_RESOURCE_FILES)
    message(STATUS "[${TUTORIAL_NAME}] Configured copying for ${NUM_RESOURCE_FILES} specific resource(s).")
else()
    message(STATUS "[${TUTORIAL_NAME}] No specific resources found to copy.")
endif()

# --- 可选：特定于目标的编译定义等 ---
# target_compile_definitions(${TUTORIAL_NAME} PRIVATE ...)

message(STATUS "[${TUTORIAL_NAME}] CMake configuration finished.")

