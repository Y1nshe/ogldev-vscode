# 定义 ogldev 静态库
add_library(ogldev STATIC)

# --- 新增：查找并配置 FreeGLUT ---
find_package(GLUT REQUIRED) # 查找 FreeGLUT 库
if(MINGW)
    # 对于 MinGW 静态链接，通常需要定义 FREEGLUT_STATIC
    target_compile_definitions(ogldev PRIVATE FREEGLUT_STATIC)
    message(STATUS "[ogldev] Defined FREEGLUT_STATIC for MinGW build.")
endif()
# --- /新增 ---

# 添加源文件 (路径相对于项目根目录 CMAKE_SOURCE_DIR)
target_sources(ogldev PRIVATE
    ${CMAKE_SOURCE_DIR}/src/common/3rdparty/stb_image.cpp
    ${CMAKE_SOURCE_DIR}/src/common/camera.cpp
    ${CMAKE_SOURCE_DIR}/src/common/cubemap_texture.cpp
    ${CMAKE_SOURCE_DIR}/src/common/glut_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/common/io_buffer.cpp
    ${CMAKE_SOURCE_DIR}/src/common/math_3d.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_app.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_atb.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_base_app.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_base_app2.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_basic_glfw_camera.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_basic_lighting.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_basic_mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_bezier_curve_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_billboard_list.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_billboard_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_flat_passthru_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_framebuffer.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_glfw.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_glfw_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_glfw_camera_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_glm_camera.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_guitex_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_gui_texture.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_infinite_grid.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_infinite_grid_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_new_lighting.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_passthru_vec2_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_phong_renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_quad_tess_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_screen_quad.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_shadow_cube_map_fbo.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_shadow_mapping_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_shadow_mapping_technique_point_light.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_shadow_map_fbo.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_shadow_map_offset_texture.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skinned_mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skinning_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skybox.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skybox_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skydome.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_skydome_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_sprite_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_stb_image.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_texture.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_tex_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_util.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ogldev_world_transform.cpp
    ${CMAKE_SOURCE_DIR}/src/common/pipeline.cpp
    ${CMAKE_SOURCE_DIR}/src/common/random_texture.cpp
    ${CMAKE_SOURCE_DIR}/src/common/technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/Techniques/ogldev_color_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/Techniques/ogldev_ray_marching_technique.cpp
    ${CMAKE_SOURCE_DIR}/src/common/Techniques/ogldev_square_vs.cpp
    # 添加 freetypeGL.cpp 到 ogldev 库
    ${CMAKE_SOURCE_DIR}/src/common/FreetypeGL/freetypeGL.cpp
    # DemoLITION 依赖暂时移除，如果需要，后续单独处理 DemoLITION 项目
    # src/../../DemoLITION/Framework/Source/core_model.cpp
)

# 添加包含目录 (对外暴露根目录下的 Include)
# 也添加 MeshOptimizer 的包含目录 (如果 find_package 定义了)
# 也添加 GLUT 的包含目录 (如果 find_package 定义了)
target_include_directories(ogldev
    PUBLIC ${CMAKE_SOURCE_DIR}/Include
    PUBLIC ${GLUT_INCLUDE_DIRS} # 新增：确保能找到 GLUT 头文件
    PRIVATE ${CMAKE_SOURCE_DIR}/common
    PRIVATE ${MeshOptimizer_INCLUDE_DIRS} # Add include dirs for MeshOptimizer
)

# 链接依赖库 (这些库由根 CMakeLists.txt 找到)
# 使用 PUBLIC 使得链接了 ogldev 的目标也能自动链接这些库并获取 include 路径
# 使用 PRIVATE 对于内部实现细节 (如 GLEW，如果教程会单独链接它)
target_link_libraries(ogldev PUBLIC
    GLUT::GLUT          # 新增：链接 FreeGLUT (使用CMake目标)
    # 或者使用 ${GLUT_LIBRARIES} 如果 FindGLUT.cmake 没有定义目标
    OpenGL::GL          # Modern OpenGL target
    ${GLFW_LIBRARIES}   # Link against the GLFW libraries variable
    glm::glm            # GLM target (CMake >= 3.11) or GLM::GLM
    assimp::assimp      # Assimp target
    Freetype::Freetype  # FreeType target
    ${MeshOptimizer_LIBRARIES} # Use MeshOptimizer libraries variable
    Fontconfig::Fontconfig # Fontconfig target
    Vulkan::Vulkan      # Vulkan target (if needed by common, otherwise move to specific tutorials)
    imgui_lib           # Link ogldev to imgui_lib as it uses its headers
    # PkgConfig::PKGCONFIG # Don't link against the pkg-config tool itself
 PRIVATE
    GLEW::glew          # 改为 PRIVATE，让教程根据需要自行链接
)

# 可能需要的编译器定义 (已在根目录添加 _USE_MATH_DEFINES 和 GLEW_STATIC)
# 已在上面添加 FREEGLUT_STATIC for MINGW
# target_compile_definitions(ogldev PUBLIC ...)

message(STATUS "CMakeLists.txt for ogldev library updated to use sources/includes from root Common/Include.")

# 注意：你需要确保根 CMakeLists.txt 中的 find_package 命令能够成功找到所有这些库。
# pkg-config 通常能帮助 CMake 在 MSYS2/MINGW64 环境下找到它们。