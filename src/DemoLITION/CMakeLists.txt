# CMakeLists.txt for the DemoLITION Framework Library

# Find all source files in the Source directory
file(GLOB DEMOLITION_SOURCES CONFIGURE_DEPENDS "Source/*.cpp" "Source/**/*.cpp") # Include subdirectories if any

# Define the library target
add_library(demolition_lib STATIC ${DEMOLITION_SOURCES})

# Specify public include directory
# Targets linking against demolition_lib will automatically get this include path
target_include_directories(demolition_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:Include> # Assuming installation might be needed later
)

# 声明 demolition_lib 依赖于 ogldev 和 imgui_lib
# 这将自动添加这些库的 include 目录到 demolition_lib 的编译中
target_link_libraries(demolition_lib PUBLIC
    ogldev
    imgui_lib
    meshoptimizer # 添加 meshoptimizer 依赖
)

# Optional: Add dependencies if DemoLITION itself depends on other libs (e.g., Common)
# target_link_libraries(demolition_lib PUBLIC Common) # Example

message(STATUS "[DemoLITION] Configured library target 'demolition_lib'")