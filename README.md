Demo的CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(Demo LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Windows SDK 和 DirectX（MinGW 通常自带 d3d11 和 dxguid）
# 无需额外 find_package，直接链接即可

# ImGui 源文件
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
)

# 主程序
set(SOURCES
    src/demo.cpp
    ${IMGUI_SOURCES}
)

# 添加可执行文件
add_executable(Demo ${SOURCES})

# 包含目录
target_include_directories(Demo PRIVATE
    ${IMGUI_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# 链接 Windows 和 DirectX 11 库（MinGW 兼容）
target_link_libraries(Demo
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    gdi32
    shell32
)

# 启用 Unicode（推荐）
target_compile_definitions(Demo PRIVATE UNICODE _UNICODE)

# 设置子系统为 Windows（如果你不需要控制台，可选）
# 如果你希望有控制台输出用于调试，先不要加 WIN32
# add_executable(MyRelaxImGUI WIN32 ${SOURCES})  # ← 无控制台
```

