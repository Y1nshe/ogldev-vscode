cmake_minimum_required(VERSION 3.16) # 提高版本要求以使用更现代的 CMake 功能

# 定义教程名称和编号 (方便引用)
set(TUTORIAL_NAME Tutorial57_ComputeShaderParticles)
set(TUTORIAL_NUM 57)

# --- 源文件 ---
# 列出此教程特定的源文件 (相对于此 CMakeLists.txt)
set(TUTORIAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/particles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/particles_technique.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tutorial57.cpp
)

# --- 资源文件 ---
# 列出需要复制到构建目录的资源文件 (相对于此 CMakeLists.txt)
set(RESOURCE_FILES
    
)

# --- 依赖项列表 (用于检查是否需要 DemoLITION 资源) ---
set(DEPENDENCIES_LIST GLEW::glew
    assimp::assimp
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
    message(STATUS "[${TUTORIAL_NAME}] Depends on demolition_lib, copying required shaders.")
    set(DEMOLITION_SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/src/DemoLITION/Shaders/GL") # 假设 shaders 在这里
    set(DEMOLITION_SHADER_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TUTORIAL_NAME}/Framework/Shaders/GL") # 目标路径

    # 查找需要复制的 DemoLITION 着色器
    file(GLOB DEMOLITION_SHADER_FILES CONFIGURE_DEPENDS
        "${DEMOLITION_SHADER_SRC_DIR}/forward_lighting.vs"
        "${DEMOLITION_SHADER_SRC_DIR}/forward_lighting.fs"
        # 在这里添加其他 DemoLITION 需要的着色器...
    )

    if(DEMOLITION_SHADER_FILES)
        set(COPIED_DEMOLITION_SHADERS "")
        # 确保目标子目录存在
        file(MAKE_DIRECTORY "${DEMOLITION_SHADER_DEST_DIR}")

        foreach(SHADER_ABS_PATH ${DEMOLITION_SHADER_FILES})
            get_filename_component(SHADER_FILENAME ${SHADER_ABS_PATH} NAME)
            set(SHADER_DEST_PATH "${DEMOLITION_SHADER_DEST_DIR}/${SHADER_FILENAME}")

            # 添加自定义命令，仅在文件更改时复制
            add_custom_command(
                OUTPUT ${SHADER_DEST_PATH}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_ABS_PATH} ${SHADER_DEST_PATH}
                DEPENDS ${SHADER_ABS_PATH}
                COMMENT "Copying DemoLITION shader ${SHADER_FILENAME} for ${TUTORIAL_NAME}"
                VERBATIM
            )
            list(APPEND COPIED_DEMOLITION_SHADERS ${SHADER_DEST_PATH})
        endforeach()

        # 创建一个聚合目标来管理 DemoLITION 资源的复制
        add_custom_target(CopyDemoLITIONShaders_${TUTORIAL_NAME} ALL DEPENDS ${COPIED_DEMOLITION_SHADERS})
        # 使可执行文件依赖于 DemoLITION 资源复制目标
        add_dependencies(${TUTORIAL_NAME} CopyDemoLITIONShaders_${TUTORIAL_NAME})
        message(STATUS "[${TUTORIAL_NAME}] Configured copying for DemoLITION shaders.")
    else()
        message(WARNING "[${TUTORIAL_NAME}] Could not find DemoLITION shaders in ${DEMOLITION_SHADER_SRC_DIR}.")
    endif()
else()
     message(STATUS "[${TUTORIAL_NAME}] Does not depend on demolition_lib, skipping DemoLITION shader copy.")
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

