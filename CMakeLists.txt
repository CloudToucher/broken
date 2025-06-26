cmake_minimum_required(VERSION 3.16)
project(broken LANGUAGES CXX)
# 启用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编码
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 编译器优化设置
if(MSVC)
    # Debug模式设置
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1 /MDd")
    
    # Release模式设置 - 最大优化
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MD")
    
    # RelWithDebInfo模式 - 优化+调试信息
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob1 /Zi /DNDEBUG /MD")
    
    # 额外优化选项
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/Ot /Oi /Gy /GL)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC/Clang 优化设置
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
endif()
# 输出目录结构
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 检测操作系统和架构
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_DIR "x64")
    else()
        set(PLATFORM_DIR "x86")
    endif()
elseif(APPLE)
    set(PLATFORM_DIR "macos")
elseif(UNIX)
    set(PLATFORM_DIR "linux")
endif()

# SDL 路径设置 - 支持多平台
set(SDL3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL3")
set(SDL3_TTF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL3_ttf")

# 头文件路径（包含 nlohmann/json、SDL3、SDL3_ttf）
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL3_ROOT}/include
    ${SDL3_TTF_ROOT}/include
)

# 根据平台设置库文件路径
if(WIN32)
    # Windows 平台
    link_directories(${SDL3_ROOT}/lib)
    link_directories(${SDL3_TTF_ROOT}/lib)
    
    # 设置库文件名
    set(SDL3_LIBRARIES SDL3.lib)
    set(SDL3_TTF_LIBRARIES SDL3_ttf.lib)
    
    # DLL 文件路径
    set(SDL3_DLL "${SDL3_ROOT}/bin/SDL3.dll")
    set(SDL3_TTF_DLL "${SDL3_TTF_ROOT}/bin/SDL3_ttf.dll")
elseif(APPLE)
    # macOS 平台
    find_library(SDL3_LIBRARIES SDL3 PATHS ${SDL3_ROOT}/lib)
    find_library(SDL3_TTF_LIBRARIES SDL3_ttf PATHS ${SDL3_TTF_ROOT}/lib)
elseif(UNIX)
    # Linux 平台
    find_library(SDL3_LIBRARIES SDL3 PATHS ${SDL3_ROOT}/lib)
    find_library(SDL3_TTF_LIBRARIES SDL3_ttf PATHS ${SDL3_TTF_ROOT}/lib)
endif()

# 自动收集源文件
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

# 创建可执行文件
add_executable(broken ${SOURCES})

# 链接 SDL3 和 SDL3_ttf 库
target_link_libraries(broken
    ${SDL3_LIBRARIES}
    ${SDL3_TTF_LIBRARIES}
)

# 平台特定的后处理命令
if(WIN32)
    # Windows: 复制 DLL 文件
    if(EXISTS "${SDL3_DLL}")
        add_custom_command(TARGET broken POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL3_DLL}"
                "$<TARGET_FILE_DIR:broken>"
        )
    endif()
    
    if(EXISTS "${SDL3_TTF_DLL}")
        add_custom_command(TARGET broken POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL3_TTF_DLL}"
                "$<TARGET_FILE_DIR:broken>"
        )
    endif()
endif()

# 自动复制资源文件夹（assets, map, jsons）
foreach(dir assets map jsons)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${dir}")
        add_custom_command(TARGET broken POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/${dir}"
                "$<TARGET_FILE_DIR:broken>/${dir}"
        )
    endif()
endforeach()

# 打印配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "SDL3 Root: ${SDL3_ROOT}")
message(STATUS "SDL3_TTF Root: ${SDL3_TTF_ROOT}")