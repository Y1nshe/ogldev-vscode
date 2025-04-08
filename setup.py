import os
import shutil
import xml.etree.ElementTree as ET
import re
import sys

# --- 配置 ---
VS_PROJECTS_BASE_DIR = "Windows/ogldev_vs_2022" # VS 项目文件所在的基础目录
TARGET_SRC_BASE_DIR = "src"                     # CMake 源文件生成的目标基础目录
PROJECT_ROOT = os.getcwd()                      # 假设脚本在项目根目录运行
COMMON_DIR_NAME = "Common"                      # 原始项目中通用代码的目录名
MAIN_INCLUDE_DIR_NAME = "Include"               # 主包含目录名 (其下的文件不复制)
RESOURCE_EXTENSIONS = {".fs", ".vs", ".glsl"}   # 需要复制的资源文件扩展名 (小写) - 添加了 .glsl
# 配置用于解析依赖项的 VS 配置 (尝试查找此配置下的 <AdditionalDependencies>)
VS_DEPENDENCY_CONFIG_CONDITION = "'$(Configuration)|$(Platform)'=='Debug|x64'"
# --- /配置 ---

# --- CMakeLists.txt 模板 ---
CMAKE_TEMPLATE = """\
cmake_minimum_required(VERSION 3.16) # 提高版本要求以使用更现代的 CMake 功能

# 定义教程名称和编号 (方便引用)
set(TUTORIAL_NAME {target_name})
set(TUTORIAL_NUM {tutorial_num})

# --- 源文件 ---
# 列出此教程特定的源文件 (相对于此 CMakeLists.txt)
set(TUTORIAL_SOURCES
    {sources_list}
)

# --- 资源文件 ---
# 列出需要复制到构建目录的资源文件 (相对于此 CMakeLists.txt)
set(RESOURCE_FILES
    {resources_list}
)

# --- 依赖项列表 (用于检查是否需要 DemoLITION 资源) ---
set(DEPENDENCIES_LIST {dependencies_list})

# --- 定义可执行目标 ---
add_executable(${{TUTORIAL_NAME}} ${{TUTORIAL_SOURCES}})

# --- 新增：设置目标的可执行文件输出目录 ---
# 将此教程的可执行文件放在 bin/<TutorialName> 子目录下
set_target_properties(${{TUTORIAL_NAME}} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}/${{TUTORIAL_NAME}}")

# --- 设置包含目录 ---
# 确保可以找到此目录中复制过来的头文件
target_include_directories(${{TUTORIAL_NAME}} PRIVATE ${{CMAKE_CURRENT_SOURCE_DIR}})

# --- 链接库 ---
# 基础库 + 教程特定库
target_link_libraries(${{TUTORIAL_NAME}} PRIVATE
    ogldev      # 我们的核心库
    imgui_lib   # ImGui 集成库
    ${{DEPENDENCIES_LIST}} # 动态添加的其他依赖
)

# --- DemoLITION 资源处理 (如果需要) ---
list(FIND DEPENDENCIES_LIST "demolition_lib" _demolition_index)
if(_demolition_index GREATER -1)
    message(STATUS "[${{TUTORIAL_NAME}}] Depends on demolition_lib, copying required shaders.")
    set(DEMOLITION_SHADER_SRC_DIR "${{CMAKE_SOURCE_DIR}}/src/DemoLITION/Shaders/GL") # 假设 shaders 在这里
    set(DEMOLITION_SHADER_DEST_DIR "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}/${{TUTORIAL_NAME}}/Framework/Shaders/GL") # 目标路径

    # 查找需要复制的 DemoLITION 着色器
    file(GLOB DEMOLITION_SHADER_FILES CONFIGURE_DEPENDS
        "${{DEMOLITION_SHADER_SRC_DIR}}/forward_lighting.vs"
        "${{DEMOLITION_SHADER_SRC_DIR}}/forward_lighting.fs"
        # 在这里添加其他 DemoLITION 需要的着色器...
    )

    if(DEMOLITION_SHADER_FILES)
        set(COPIED_DEMOLITION_SHADERS "")
        # 确保目标子目录存在
        file(MAKE_DIRECTORY "${{DEMOLITION_SHADER_DEST_DIR}}")

        foreach(SHADER_ABS_PATH ${{DEMOLITION_SHADER_FILES}})
            get_filename_component(SHADER_FILENAME ${{SHADER_ABS_PATH}} NAME)
            set(SHADER_DEST_PATH "${{DEMOLITION_SHADER_DEST_DIR}}/${{SHADER_FILENAME}}")

            # 添加自定义命令，仅在文件更改时复制
            add_custom_command(
                OUTPUT ${{SHADER_DEST_PATH}}
                COMMAND ${{CMAKE_COMMAND}} -E copy_if_different ${{SHADER_ABS_PATH}} ${{SHADER_DEST_PATH}}
                DEPENDS ${{SHADER_ABS_PATH}}
                COMMENT "Copying DemoLITION shader ${{SHADER_FILENAME}} for ${{TUTORIAL_NAME}}"
                VERBATIM
            )
            list(APPEND COPIED_DEMOLITION_SHADERS ${{SHADER_DEST_PATH}})
        endforeach()

        # 创建一个聚合目标来管理 DemoLITION 资源的复制
        add_custom_target(CopyDemoLITIONShaders_${{TUTORIAL_NAME}} ALL DEPENDS ${{COPIED_DEMOLITION_SHADERS}})
        # 使可执行文件依赖于 DemoLITION 资源复制目标
        add_dependencies(${{TUTORIAL_NAME}} CopyDemoLITIONShaders_${{TUTORIAL_NAME}})
        message(STATUS "[${{TUTORIAL_NAME}}] Configured copying for DemoLITION shaders.")
    else()
        message(WARNING "[${{TUTORIAL_NAME}}] Could not find DemoLITION shaders in ${{DEMOLITION_SHADER_SRC_DIR}}.")
    endif()
else()
     message(STATUS "[${{TUTORIAL_NAME}}] Does not depend on demolition_lib, skipping DemoLITION shader copy.")
endif()

# --- 教程特定资源文件处理 ---
if(RESOURCE_FILES)
    # 为资源文件创建一个目标，以便在需要时复制它们
    set(COPIED_RESOURCES "")
    foreach(RESOURCE_REL_PATH ${{RESOURCE_FILES}})
        # 获取绝对路径以进行复制
        get_filename_component(RESOURCE_ABS_PATH ${{CMAKE_CURRENT_SOURCE_DIR}}/${{RESOURCE_REL_PATH}} ABSOLUTE)
        # 获取文件名以用作目标路径
        get_filename_component(RESOURCE_FILENAME ${{RESOURCE_REL_PATH}} NAME)
        # 修改：将目标路径设置为可执行文件输出目录下的教程子目录 (build/bin/<TutorialName>)
        set(DESTINATION_PATH "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}/${{TUTORIAL_NAME}}/${{RESOURCE_FILENAME}}")

        # 添加自定义命令，仅在文件更改时复制
        add_custom_command(
            OUTPUT ${{DESTINATION_PATH}}
            COMMAND ${{CMAKE_COMMAND}} -E copy_if_different ${{RESOURCE_ABS_PATH}} ${{DESTINATION_PATH}}
            DEPENDS ${{RESOURCE_ABS_PATH}}  # 依赖于源文件
            COMMENT "Copying ${{RESOURCE_FILENAME}} for ${{TUTORIAL_NAME}}"
            VERBATIM
        )
        list(APPEND COPIED_RESOURCES ${{DESTINATION_PATH}})
    endforeach()

    # 创建一个聚合目标来管理所有资源的复制
    add_custom_target(CopyResources_${{TUTORIAL_NAME}} ALL DEPENDS ${{COPIED_RESOURCES}})

    # 使可执行文件依赖于资源复制目标
    add_dependencies(${{TUTORIAL_NAME}} CopyResources_${{TUTORIAL_NAME}})

    # 获取资源文件的数量
    list(LENGTH RESOURCE_FILES NUM_RESOURCE_FILES)
    message(STATUS "[${{TUTORIAL_NAME}}] Configured copying for ${{NUM_RESOURCE_FILES}} specific resource(s).")
else()
    message(STATUS "[${{TUTORIAL_NAME}}] No specific resources found to copy.")
endif()

# --- 可选：特定于目标的编译定义等 ---
# target_compile_definitions(${{TUTORIAL_NAME}} PRIVATE ...)

message(STATUS "[${{TUTORIAL_NAME}}] CMake configuration finished.")

"""
# --- /CMakeLists.txt 模板 ---

# --- 依赖库映射 ---
# MSVC .lib name -> CMake target or MinGW library name
# 注意：键应为小写
MSVC_TO_CMAKE_DEP_MAP = {
    # 恢复使用 CMake FindGLUT 目标，因为 ogldev 库现在负责链接它
    "freeglut.lib": "GLUT::GLUT",
    "freeglutd.lib": "GLUT::GLUT", # Map debug version too
    "glew32.lib": "GLEW::glew",
    "glew32d.lib": "GLEW::glew", # Map debug version too
    # Assimp - 假设你使用 find_package(assimp) 并链接 assimp::assimp
    "assimp-vc140-mt.lib": "assimp::assimp",
    "assimp-vc141-mt.lib": "assimp::assimp",
    "assimp-vc142-mt.lib": "assimp::assimp",
    "assimp-vc143-mt.lib": "assimp::assimp",
    # GLFW - map to the CMake imported target name (通常是 'glfw' 或 'GLFW::glfw')
    "glfw3.lib": "glfw", # 修改：使用目标名而非变量
    "glfw3dll.lib": "glfw", # 修改：使用目标名而非变量
    # 添加 DemoLITION 框架的映射
    "demolition_framework.lib": "demolition_lib",
    # 添加 freetype-gl 的映射
    "freetype-gl.lib": "freetype-gl",
    # 添加其他需要的映射...
}

# 过滤掉这些常见的 Windows 库 (CMake/MinGW 通常会自动处理)
WINDOWS_LIBS_TO_IGNORE = {
    "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib",
    "comdlg32.lib", "advapi32.lib", "shell32.lib", "ole32.lib",
    "oleaut32.lib", "uuid.lib", "odbc32.lib", "odbccp32.lib",
    "ws2_32.lib", # 网络库，可能需要根据情况处理
    # 添加其他可以安全忽略的库...
    # 忽略我们自己定义的库，因为它们通过 CMake 目标链接
    "ogldev.lib",
    "libogldev.lib",
    "ogldevd.lib",
    "libogldevd.lib",
}
# --- /依赖库映射 ---


def log(message, level="INFO"):
    print(f"[{level}] {message}")

def get_root_relative_path(vcxproj_dir, include_path):
    """
    将 vcxproj 文件中的相对路径转换为项目根目录的相对路径。
    假定 vcxproj 在类似 'Windows/ogldev_vs_2022/tutorialXX' 的 3 级目录下。
    """
    normalized_path = include_path.replace("\\", "/")
    if normalized_path.startswith("../../../"):
        return normalized_path[len("../../../"):]
    else:
        # 对非标准路径发出警告，但仍尝试处理（可能不适用于所有情况）
        log(f"Unexpected path format '{include_path}' relative to '{vcxproj_dir}'. Assuming it starts from project root.", "WARN")
        # 尝试移除 './' 或 '../' (如果存在于非预期位置)
        if normalized_path.startswith("./"):
            normalized_path = normalized_path[2:]
        # 注意：这个简单的处理可能不足以覆盖所有边缘情况
        return normalized_path

def parse_vcxproj(vcxproj_path):
    """解析 .vcxproj 文件查找源/头/依赖项，并扫描教程目录查找头文件和资源文件。"""
    sources = []
    resources = [] # Will store root-relative paths
    headers = []   # Will store root-relative paths
    raw_dependencies = []
    vcxproj_dir = os.path.dirname(vcxproj_path)
    # 确定教程相对于项目根目录的源目录 (假设 vcxproj 在 Windows/ogldev_vs_2022/TutorialXX/ 内)
    # ../../../ 将指向根目录
    potential_tutorial_src_dir = os.path.normpath(os.path.join(vcxproj_dir, "../../..", os.path.basename(vcxproj_dir)))
    project_root_from_vcxproj = os.path.normpath(os.path.join(vcxproj_dir, "../../..")) # Get the assumed project root

    # 实际教程源目录名 (可能与 vcxproj 所在目录名不同, 比如源在 tutorialXX 而 vcxproj 在 Windows/.../TutorialXX)
    # 我们需要从 ClCompile/ClInclude 的路径中推断
    inferred_tutorial_src_dir_name = None

    try:
        tree = ET.parse(vcxproj_path)
        root = tree.getroot()
        ns = {'msb': 'http://schemas.microsoft.com/developer/msbuild/2003'}

        # 查找 C/C++ 源文件 (<ClCompile>)
        for item in root.findall('.//msb:ClCompile', ns):
            include_path = item.get('Include')
            if include_path:
                rel_path = get_root_relative_path(vcxproj_dir, include_path)
                if not rel_path.lower().startswith(f"{COMMON_DIR_NAME.lower()}/"):
                    sources.append(rel_path)
                    # 从第一个非 Common 源文件推断教程目录名
                    if inferred_tutorial_src_dir_name is None:
                         path_parts = os.path.normpath(rel_path).split(os.sep)
                         if len(path_parts) > 1: # 确保不是根目录下的文件
                             inferred_tutorial_src_dir_name = path_parts[0]

        # 查找头文件 (<ClInclude>)
        for item in root.findall('.//msb:ClInclude', ns):
            include_path = item.get('Include')
            if include_path:
                rel_path = get_root_relative_path(vcxproj_dir, include_path)
                if (not rel_path.lower().startswith(f"{COMMON_DIR_NAME.lower()}/") and
                        not rel_path.lower().startswith(f"{MAIN_INCLUDE_DIR_NAME.lower()}/")):
                    headers.append(rel_path)
                     # 从第一个非 Common/Include 头文件推断教程目录名 (如果尚未确定)
                    if inferred_tutorial_src_dir_name is None:
                         path_parts = os.path.normpath(rel_path).split(os.sep)
                         if len(path_parts) > 1:
                             inferred_tutorial_src_dir_name = path_parts[0]

        # ----- 新增/修改：扫描推断出的教程源目录查找资源文件和头文件 -----
        if inferred_tutorial_src_dir_name:
            actual_tutorial_src_dir = os.path.join(PROJECT_ROOT, inferred_tutorial_src_dir_name)
            log(f"Scanning for headers and resources in inferred source directory: {actual_tutorial_src_dir}", "DEBUG")
            if os.path.isdir(actual_tutorial_src_dir):
                # 定义头文件扩展名
                header_extensions = {".h", ".hpp"}
                for root_dir, _, files in os.walk(actual_tutorial_src_dir):
                    for file in files:
                        file_name, ext = os.path.splitext(file)
                        ext_lower = ext.lower()
                        # 计算相对于项目根目录的路径
                        full_path = os.path.join(root_dir, file)
                        file_rel_path = os.path.relpath(full_path, PROJECT_ROOT).replace("\\", "/")

                        # 检查是否为资源文件
                        if ext_lower in RESOURCE_EXTENSIONS:
                            # 确保不在 Common 下且未被 vcxproj 处理过
                            if file_rel_path not in resources and not file_rel_path.lower().startswith(f"{COMMON_DIR_NAME.lower()}/"):
                                resources.append(file_rel_path)
                                log(f"Found resource by scanning: {file_rel_path}", "DEBUG")
                        # 检查是否为头文件
                        elif ext_lower in header_extensions:
                            # 确保不在 Common/Include 下且未被 vcxproj 处理过
                            if file_rel_path not in headers and \
                               not file_rel_path.lower().startswith(f"{COMMON_DIR_NAME.lower()}/") and \
                               not file_rel_path.lower().startswith(f"{MAIN_INCLUDE_DIR_NAME.lower()}/"):
                                headers.append(file_rel_path)
                                log(f"Found header by scanning: {file_rel_path}", "DEBUG")
            else:
                 log(f"Inferred tutorial source directory not found or is not a directory: {actual_tutorial_src_dir}", "WARN")
        else:
            log("Could not infer tutorial source directory name from vcxproj paths. Header/Resource scanning skipped.", "WARN")
        # ----- /新增/修改 -----

        # [保留] 原有的通过 vcxproj 查找资源文件的逻辑 (以防扫描失败或遗漏)
        # 注意：如果扫描找到了文件，这里的逻辑可能不会再添加重复文件
        # 查找资源文件 (<None>, <Image>, <Media>, <Content> 等)
        for item in root.findall('.//msb:None', ns): # 可以扩展到其他标签
            include_path = item.get('Include')
            if include_path:
                rel_path = get_root_relative_path(vcxproj_dir, include_path)
                _, ext = os.path.splitext(rel_path)
                # 确保是资源文件，不在Common下，并且之前没有通过扫描找到
                if ext.lower() in RESOURCE_EXTENSIONS and \
                   not rel_path.lower().startswith(f"{COMMON_DIR_NAME.lower()}/") and \
                   rel_path not in resources:
                    resources.append(rel_path)
                    log(f"Found resource via vcxproj <None>: {rel_path}", "DEBUG")


        # 查找依赖项 (<AdditionalDependencies>)
        # 尝试找到特定配置 (如 'Debug|x64') 的 ItemDefinitionGroup
        found_deps = False
        for group in root.findall('.//msb:ItemDefinitionGroup', ns):
            condition = group.get('Condition', '')
            if condition == VS_DEPENDENCY_CONFIG_CONDITION:
                link_node = group.find('.//msb:Link', ns)
                if link_node is not None:
                    deps_node = link_node.find('.//msb:AdditionalDependencies', ns)
                    if deps_node is not None and deps_node.text:
                        # 分割字符串，去除空条目，并过滤掉所有以 $ 或 % 开头的宏/变量
                        raw_dependencies = [
                            dep.strip() for dep in deps_node.text.split(';')
                            if dep.strip() and not dep.strip().startswith('$') and not dep.strip().startswith('%')
                        ]
                        found_deps = True
                        break # 找到就退出

        if not found_deps:
            log(f"Could not find <AdditionalDependencies> for condition {VS_DEPENDENCY_CONFIG_CONDITION} in {vcxproj_path}", "WARN")
            # 可以添加备用逻辑，例如查找第一个找到的 Link 节点

    except ET.ParseError as e:
        log(f"Error parsing XML file {vcxproj_path}: {e}", "ERROR")
    except FileNotFoundError:
        log(f"File not found {vcxproj_path}", "ERROR")

    # 使用 set 去重后转回 list，以防扫描和 vcxproj 解析找到相同文件
    return sorted(list(set(sources))), sorted(list(set(resources))), sorted(list(set(headers))), raw_dependencies

def map_msvc_to_cmake_deps(raw_deps):
    """将 MSVC 的 .lib 列表映射到 CMake/MinGW 库列表。"""
    cmake_deps = set() # 使用集合避免重复
    unmapped_deps = []

    for dep in raw_deps:
        # 在任何处理之前，首先过滤掉所有以 $ 或 % 开头的依赖项
        dep_stripped = dep.strip()
        if dep_stripped.startswith('$') or dep_stripped.startswith('%'):
            log(f"Ignoring potential MSBuild macro/variable: '{dep}'", "DEBUG")
            continue

        dep_lower = dep_stripped.lower()
        if dep_lower in WINDOWS_LIBS_TO_IGNORE:
            continue
        elif dep_lower in MSVC_TO_CMAKE_DEP_MAP:
            cmake_deps.add(MSVC_TO_CMAKE_DEP_MAP[dep_lower])
        else:
            # 尝试通用规则：移除 .lib 后缀
            if dep_lower.endswith(".lib"):
                potential_name = dep_stripped[:-4] # 使用未转小写的原始名称去除后缀
                # 如果需要，可以在这里添加更复杂的规则或前缀 (如 'lib')
                cmake_deps.add(potential_name)
                log(f"Applying generic mapping for '{dep}' -> '{potential_name}'. Verify this is correct for MinGW/CMake.", "DEBUG")
            else:
                # 无法识别或映射
                unmapped_deps.append(dep)
                cmake_deps.add(dep) # 暂时按原样添加，并发出警告
                log(f"Unmapped dependency '{dep}'. Adding as is. Verify linking.", "WARN")

    return sorted(list(cmake_deps)) # 返回排序后的列表

def process_tutorials():
    """处理所有教程的 vcxproj 文件，创建目录，复制文件，生成 CMakeLists.txt。"""
    add_subdirectory_commands = []
    processed_count = 0
    skipped_count = 0

    if not os.path.isdir(VS_PROJECTS_BASE_DIR):
        log(f"Visual Studio projects base directory not found: {VS_PROJECTS_BASE_DIR}", "ERROR")
        return

    # 正则表达式查找 tutorialXX 或 tutorialXX 加上下划线和描述符的目录
    tutorial_dir_pattern = re.compile(r"^(tutorial(\d+)(_[a-zA-Z0-9_]+)?)$", re.IGNORECASE)

    # 遍历 VS 项目目录下的所有条目
    vs_project_dirs = sorted(os.listdir(VS_PROJECTS_BASE_DIR))
    log(f"Found {len(vs_project_dirs)} potential items in {VS_PROJECTS_BASE_DIR}")

    for item_name in vs_project_dirs:
        vs_tutorial_dir = os.path.join(VS_PROJECTS_BASE_DIR, item_name)
        match = tutorial_dir_pattern.match(item_name)

        # 确保是目录并且匹配模式 "tutorialXX" 或 "tutorialXX_youtube"
        if os.path.isdir(vs_tutorial_dir) and match:
            # 使用匹配到的完整名称作为教程名 (例如 tutorial01, tutorial12_youtube)
            # tutorial_name = match.group(1)
            # 或者，如果我们总是希望目标目录不带 _youtube 后缀？
            # 这需要决定如何处理原始带有 _youtube 的源目录
            # 暂时：目标目录名与 VS 项目目录名保持一致
            tutorial_name = item_name
            tutorial_num_str = match.group(2).zfill(2) # 提取数字部分并补零

            log(f"\n--- Processing {tutorial_name} ---")

            # 查找目录中的 .vcxproj 文件
            vcxproj_files = [f for f in os.listdir(vs_tutorial_dir) if f.lower().endswith(".vcxproj")]
            if not vcxproj_files:
                log(f"No .vcxproj file found in {vs_tutorial_dir}. Skipping.", "WARN")
                skipped_count += 1
                continue
            # 选择第一个找到的 vcxproj 文件
            vcxproj_filename = vcxproj_files[0]
            vcxproj_path = os.path.join(vs_tutorial_dir, vcxproj_filename)

            # 解析 vcxproj 获取源文件、资源文件、头文件和依赖（已排除 Common 等）
            sources_rel_root, resources_rel_root, headers_rel_root, raw_dependencies = parse_vcxproj(vcxproj_path)

            if not sources_rel_root:
                 log(f"No specific source files found for {tutorial_name} (excluding Common). Skipping.", "WARN")
                 skipped_count += 1
                 continue

            log(f"  Sources (root rel): {sources_rel_root}")
            log(f"  Headers (root rel, specific): {headers_rel_root}")
            log(f"  Resources (root rel): {resources_rel_root}")
            log(f"  Raw Dependencies: {raw_dependencies}")

            # 映射依赖库
            mapped_deps = map_msvc_to_cmake_deps(raw_dependencies)
            log(f"  Mapped Dependencies: {mapped_deps}")


            # --- 文件和目录操作 ---
            target_tutorial_dir = os.path.join(TARGET_SRC_BASE_DIR, tutorial_name)
            try:
                os.makedirs(target_tutorial_dir, exist_ok=True)
                log(f"  Ensured target directory: {target_tutorial_dir}")
            except OSError as e:
                log(f"Failed to create directory {target_tutorial_dir}: {e}. Skipping.", "ERROR")
                skipped_count += 1
                continue

            copied_sources_target_rel = []
            copied_resources_target_rel = []
            copied_headers_target_rel = []

            # --- 复制文件函数 ---
            def copy_file(rel_path_list, type_name, target_rel_list):
                success_count = 0
                log_prefix = f"  Copying {type_name}:"
                skip_prefix = f"  Skipping copy (up-to-date): {type_name}"
                for rel_path in rel_path_list:
                    src_abs = os.path.join(PROJECT_ROOT, rel_path)
                    # 重要：目标文件名应与源文件名相同
                    file_name = os.path.basename(rel_path)
                    dest_abs = os.path.join(PROJECT_ROOT, target_tutorial_dir, file_name)
                    try:
                        if not os.path.exists(src_abs):
                            log(f"{type_name} file not found: {src_abs}. Skipping this file.", "WARN")
                            continue

                        # 只有当目标文件不存在或源文件更新时才复制
                        if not os.path.exists(dest_abs) or os.path.getmtime(src_abs) > os.path.getmtime(dest_abs):
                             # 使用更详细的日志
                             log(f"{log_prefix} '{rel_path}' -> '{target_tutorial_dir}/{file_name}'", "DEBUG")
                             shutil.copy2(src_abs, dest_abs) # copy2 保留元数据
                        else:
                             log(f"{skip_prefix} '{file_name}'", "DEBUG")

                        target_rel_list.append(file_name) # 相对于目标 CMakeLists.txt 的路径
                        success_count += 1
                    except Exception as e:
                        log(f"Error copying {type_name} {src_abs} to {dest_abs}: {e}", "ERROR")
                return success_count

            # --- 执行复制 ---
            copy_file(sources_rel_root, "Source", copied_sources_target_rel)
            copy_file(headers_rel_root, "Header", copied_headers_target_rel)
            copy_file(resources_rel_root, "Resource", copied_resources_target_rel)


            # --- 生成 CMakeLists.txt ---
            if not copied_sources_target_rel:
                log(f"No source files were successfully copied/found for {tutorial_name}. Skipping CMakeLists.txt generation.", "WARN")
                skipped_count += 1
                continue

            # 准备 CMake 模板的填充内容
            # 源文件列表需要 CMAKE_CURRENT_SOURCE_DIR 前缀
            cmake_sources_list = "\n    ".join(f"${{CMAKE_CURRENT_SOURCE_DIR}}/{f}" for f in sorted(copied_sources_target_rel))
            # 资源文件列表直接使用文件名
            cmake_resources_list = "\n    ".join(sorted(copied_resources_target_rel))
            # 依赖项列表，格式化为多行
            cmake_deps_list = "\n    ".join(mapped_deps) if mapped_deps else "# No specific dependencies found/mapped"

            cmake_content = CMAKE_TEMPLATE.format(
                target_name=tutorial_name, # 使用实际目录名作为目标名
                tutorial_num=tutorial_num_str, # 保留数字编号
                sources_list=cmake_sources_list,
                resources_list=cmake_resources_list,
                dependencies_list=cmake_deps_list
            )

            cmake_list_path = os.path.join(target_tutorial_dir, "CMakeLists.txt")
            try:
                # 使用 UTF-8 编码和 LF 行尾符写入
                with open(cmake_list_path, "w", encoding='utf-8', newline='\n') as f:
                    f.write(cmake_content)
                log(f"  Generated CMakeLists.txt: {cmake_list_path}")
                # 添加对应的 add_subdirectory 指令 (使用 / 作为路径分隔符)
                add_subdirectory_commands.append(f"add_subdirectory({target_tutorial_dir.replace(os.sep, '/')})")
                processed_count += 1
            except Exception as e:
                log(f"Error writing CMakeLists.txt to {cmake_list_path}: {e}", "ERROR")
                skipped_count += 1
        # else:
            # log(f"Item '{item_name}' is not a directory or doesn't match tutorial pattern.", "DEBUG")


    # --- 输出总结 ---
    log("\n--- Script Finished ---")
    log(f"Processed: {processed_count}, Skipped/Errors: {skipped_count}")
    if add_subdirectory_commands:
        log(f"\nPlease add the following lines to your root CMakeLists.txt (inside the 'if(MINGW)' block if applicable):")
        print("-" * 60)
        # -- DEBUG: Print the list before sorting --
        log(f"Commands before sorting: {add_subdirectory_commands}", "DEBUG")
        # -- END DEBUG --
        # 对命令进行排序，确保按教程编号顺序添加 (基于 tutorialXX)
        # 使用 re.IGNORECASE 忽略大小写
        add_subdirectory_commands.sort(key=lambda x: int(re.search(r'tutorial(\d+)', x, re.IGNORECASE).group(1)))
        for command in add_subdirectory_commands:
            print(command)
        print("-" * 60)
    else:
        log("No CMakeLists.txt files were generated or no tutorials processed successfully.")

# --- 主程序入口 ---
if __name__ == "__main__":
    # 设置日志级别 (例如 "DEBUG" 可以看到更多复制信息)
    log_level = os.environ.get("LOG_LEVEL", "INFO").upper()
    # (可以在这里添加更复杂的日志级别处理)

    # 确保我们在项目根目录
    if not os.path.exists("CMakeLists.txt") or not os.path.exists(VS_PROJECTS_BASE_DIR):
         print("[ERROR] This script must be run from the project root directory", file=sys.stderr)
         print(f"[ERROR] Current directory: {os.getcwd()}", file=sys.stderr)
         print(f"[ERROR] Make sure 'CMakeLists.txt' and '{VS_PROJECTS_BASE_DIR}/' exist here.", file=sys.stderr)
         sys.exit(1)

    log(f"Starting tutorial setup script. Project Root: {PROJECT_ROOT}")
    process_tutorials()
