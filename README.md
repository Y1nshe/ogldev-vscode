# ogldev-vscode

本项目基于 [ogldev](https://www.ogldev.org/index.html) 网站教程的代码库，适配了 MSYS2/MinGW-w64 环境。

**注意**: 目前主要适配了大部分 OpenGL 核心教程。以下系列**尚未**完全适配或测试：
*   Terrain 系列教程
*   Vulkan 系列教程
*   Tutorial 58 及之后（资源文件缺失）

目标是让经典的 OpenGL 部分能够顺利运行起来。

## 系统要求

*   **操作系统**: Windows
*   **构建环境**: [MSYS2](https://www.msys2.org/)

## 环境设置与依赖安装

请在 **MSYS2 MinGW 64-bit** 终端中执行以下步骤来准备环境。除此之外，本项目设置默认终端为 MSYS2 MinGW 64-bit，因此，以下操作可以直接在 VS Code 的终端中执行。

1.  **安装 MSYS2**:
    *   访问 [MSYS2 官网](https://www.msys2.org/) 下载并安装。
    *   安装完成后，请启动 **MSYS2 MinGW 64-bit** 终端。

2.  **设置 MSYS64_ROOT 环境变量 (重要)**:
    为了让 VS Code 正确找到 MSYS2/MinGW 的工具链（如 GDB 调试器和编译器），你需要设置一个系统环境变量 `MSYS64_ROOT`。
    *   **变量名**: `MSYS64_ROOT`
    *   **变量值**: 你的 MSYS64 安装根目录的**完整路径** (例如 `C:\msys64` 或 `C:/msys64`)。

    **设置方法 (Windows):**
    1.  在 Windows 搜索栏搜索 "环境变量" 并选择 "编辑系统环境变量"。
    2.  在弹出的 "系统属性" 窗口中，点击 "环境变量..." 按钮。
    3.  在 "用户变量" 区域 (推荐) 或 "系统变量" 区域，点击 "新建..."。
    4.  输入变量名 `MSYS64_ROOT` 和对应的变量值 (你的 MSYS64 安装路径)。
    5.  确定所有打开的窗口。

    配置此环境变量后，`.vscode` 目录下的 `settings.json` 和 `launch.json` 文件会使用 `${env:MSYS64_ROOT}` 来定位 MSYS2 相关工具。

3.  **更新软件包数据库和基础包**:
    建议首次打开终端，或定期执行以下命令更新系统：
    ```bash
    pacman -Syu
    ```
    根据提示，可能需要关闭并重新打开终端，再次执行 `pacman -Syu`，直至提示没有更多更新。

4.  **安装基础开发工具**:
    ```bash
    pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
    ```
    *   `mingw-w64-x86_64-toolchain`: 包含 GCC 编译器等核心工具。
    *   `mingw-w64-x86_64-cmake`: 项目构建系统。
    *   `mingw-w64-x86_64-ninja`: 一个高效的构建工具，推荐使用。

5.  **安装图形与 OpenGL 依赖**:
    ```bash
    pacman -S --needed mingw-w64-x86_64-glew \
             mingw-w64-x86_64-glfw \
             mingw-w64-x86_64-freeglut \
             mingw-w64-x86_64-glm \
             mingw-w64-x86_64-assimp \
             mingw-w64-x86_64-freetype \
             mingw-w64-x86_64-imagemagick \
             mingw-w64-x86_64-meshoptimizer
    ```

6.  **安装辅助工具与库**:
    ```bash
    pacman -S --needed mingw-w64-x86_64-dlfcn \
             mingw-w64-x86_64-pkgconf \
             mingw-w64-x86_64-fontconfig
    ```

7.  **安装 Vulkan 相关依赖**:
    ```bash
    pacman -S --needed mingw-w64-x86_64-vulkan-headers \
             mingw-w64-x86_64-vulkan-loader
    ```

## 配置与运行项目

环境配置完成后，就可以准备运行项目了。

1.  **克隆仓库**:
    ```bash
    git clone https://github.com/Y1nshe/ogldev-vscode.git
    cd ogldev
    ```

2.  **配置 CMake 环境 (VS Code)**:
    *   请确保已安装 **CMake Tools** (`ms-vscode.cmake-tools`) 插件。
    *   项目中已预设名为 `mingw64` 的 CMake Kit，该 Kit 利用了你之前设置的 `MSYS64_ROOT` 环境变量来定位 MinGW-w64 工具链 (GCC 编译器、GDB 调试器等)。
    *   CMake Tools 插件将自动检测并优先使用此 `mingw64` Kit 来配置项目。如果首次打开项目时 CMake Tools 插件询问选择 Kit，请选择 `mingw64`。
    *   配置完成后，**无需手动执行构建命令**。当你通过 VS Code 启动调试或运行时 (例如按 F5 或点击侧边栏的运行/调试按钮)，插件会自动触发构建过程。

3. **运行与调试**:
    *   通过 VS Code 配置好 CMake 环境后，可以直接使用 VS Code 的运行和调试功能 (例如按 F5 或点击侧边栏的运行按钮) 来启动教程。插件会自动在运行前完成必要的构建步骤。
    *   构建生成的可执行文件默认位于 `build/bin` 目录下。如果需要，也可以在构建完成后手动从 `build` 目录启动：
        ```bash
        ./bin/Tutorial<XX>_[Name]/Tutorial<XX>_[Name].exe
        # 例如:
        ./bin/Tutorial01/Tutorial01.exe
        ./bin/Tutorial38/Tutorial38.exe
        ```
        注意：部分教程可能需要特定的工作目录才能正确加载资源文件，请留意相关说明。

## 项目构建优势

*   **快速构建**: 使用 Ninja 作为构建工具，速度较快。
*   **增量编译**: 修改代码后，仅重新编译受影响的部分。
*   **依赖自动发现**: CMake 会尝试自动查找通过 `pacman` 安装的依赖库。

## VS Code 开发环境

推荐使用 VS Code 进行开发。为了提升体验，建议安装以下插件。项目中包含了 `.vscode/extensions.json` 文件，VS Code 会提示安装这些推荐插件：

*   **C/C++ (`ms-vscode.cpptools`)**: 提供 C/C++ 语言智能提示、调试等核心功能。
*   **Shader languages support for VS Code (`slevesque.shader`)**: GLSL 等着色器语言的语法高亮支持。
*   **CMake Tools (`ms-vscode.cmake-tools`)**: 集成 CMake 项目的配置、构建与调试。

## 许可证

本项目基于 [ogldev](http://ogldev.atspace.co.uk/) 的原始代码进行修改和适配，原始代码采用 [GNU General Public License v3.0 or later](http://www.gnu.org/licenses/gpl-3.0.html)。
因此，本项目整体同样采用 **GNU General Public License v3.0 (GPLv3)**。
你可以在项目根目录找到 [LICENSE](LICENSE) 文件查看完整的许可证文本。
