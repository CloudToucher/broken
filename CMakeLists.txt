cmake_minimum_required(VERSION 3.16)
project(broken)

# C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows控制台应用
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 输出到bin目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含头文件
include_directories(
    include
    libs/SDL3/include
    libs/SDL3_ttf/include
)

# 链接库目录
link_directories(
    libs/SDL3/lib
    libs/SDL3_ttf/lib
)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# 创建可执行文件
add_executable(broken ${SOURCES})

# 链接库
target_link_libraries(broken SDL3.lib SDL3_ttf.lib)

# 复制DLL文件
add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/libs/SDL3/bin/SDL3.dll"
        "$<TARGET_FILE_DIR:broken>"
)

add_custom_command(TARGET broken POST_BUILD  
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/libs/SDL3_ttf/bin/SDL3_ttf.dll"
        "$<TARGET_FILE_DIR:broken>"
)

# 复制资源文件夹
add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/map"
        "$<TARGET_FILE_DIR:broken>/map"
)

add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
        "${CMAKE_SOURCE_DIR}/jsons"
        "$<TARGET_FILE_DIR:broken>/jsons"
)

add_custom_command(TARGET broken POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" 
        "$<TARGET_FILE_DIR:broken>/assets"
)

# 设置启动项目
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT broken) 