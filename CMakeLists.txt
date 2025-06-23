cmake_minimum_required(VERSION 3.16)
project(broken LANGUAGES CXX)
# 启用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编码
if(MSVC)
    add_compile_options(/utf-8)
endif()
# 输出目录结构
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# SDL 路径
set(SDL3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL3")
set(SDL3_TTF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL3_ttf")

# 头文件路径（包含 nlohmann/json、SDL3、SDL3_ttf）
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL3_ROOT}/include
    ${SDL3_TTF_ROOT}/include
)

# 链接库路径
link_directories(${SDL3_ROOT}/lib)
link_directories(${SDL3_TTF_ROOT}/lib)

# 自动收集源文件
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

# 创建可执行文件
add_executable(broken ${SOURCES})

# 链接 SDL3 和 SDL3_ttf 库
target_link_libraries(broken
    SDL3.lib
    SDL3_ttf.lib
)

# 自动复制 SDL DLL
add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_ROOT}/bin/SDL3.dll"
        "$<TARGET_FILE_DIR:broken>"
)
add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_TTF_ROOT}/bin/SDL3_ttf.dll"
        "$<TARGET_FILE_DIR:broken>"
)

# 自动复制资源文件夹（assets, map, jsons）
foreach(dir assets map jsons)
    add_custom_command(TARGET broken POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${dir}"
            "$<TARGET_FILE_DIR:broken>/${dir}"
    )
endforeach()