# ============================================================================
# CMake 最低版本要求
# ============================================================================
cmake_minimum_required(VERSION 3.14)

# ============================================================================
# 项目定义
# ============================================================================
project(LogLib VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# C++ 标准设置
# ============================================================================
set(CMAKE_CXX_STANDARD 17)              # 使用 C++11 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 强制要求 C++11
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # 生成位置无关代码（用于共享库）

# ============================================================================
# 编译选项
# ============================================================================
option(BUILD_SHARED_LIBS "构建为共享库(.so/.dll)" ON)
option(BUILD_TESTS "构建单元测试" OFF)
option(BUILD_EXAMPLES "构建示例程序" OFF)

# 全局禁用 C4996
if(MSVC)
    add_compile_options(/wd4996)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/utf-8)
endif()

# ============================================================================
# 依赖管理：自动下载 spdlog
# ============================================================================
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0                      # 使用稳定版本
    GIT_SHALLOW TRUE                     # 浅克隆，减少下载时间
)
FetchContent_MakeAvailable(spdlog)

# ============================================================================
# 公共头文件列表（需要导出的头文件）
# ============================================================================
set(PUBLIC_HEADERS
    include/log_lib/Export.h            # 动态库导出宏
    include/log_lib/logger.h            # 统一入口头文件
    include/log_lib/ILogger.h           # 抽象接口定义
    include/log_lib/LoggerManager.h     # 日志管理器
    include/log_lib/LoggerMacros.h      # 便捷宏定义
    include/log_lib/spdlog/SpdlogAdapter.h  # spdlog 适配器
)

# ============================================================================
# 源文件列表
# ============================================================================
set(SOURCES
    src/logger.cpp                      # 统一入口 API 实现
    src/LoggerManager.cpp               # 管理器实现
    src/spdlog/SpdlogAdapter.cpp        # spdlog 适配器实现
)

# ============================================================================
# 创建库目标
# ============================================================================
add_library(log_lib ${SOURCES} ${PUBLIC_HEADERS})

# 设置头文件搜索路径
target_include_directories(log_lib
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>        # 安装后路径
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src                         # 私有源文件路径
)

# 链接依赖库
target_link_libraries(log_lib PRIVATE spdlog::spdlog)  # 链接 spdlog

# 查找并链接线程库
find_package(Threads REQUIRED)
target_link_libraries(log_lib PRIVATE Threads::Threads)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(log_lib PRIVATE LOG_LIB_BUILD_SHARED)
    target_compile_definitions(log_lib PUBLIC LOG_LIB_SHARED)
endif()

# ============================================================================
# 设置库属性
# ============================================================================
set_target_properties(log_lib PROPERTIES
    VERSION ${PROJECT_VERSION}          # 库版本号
    SOVERSION 1                         # 二进制兼容版本号
    PUBLIC_HEADER "${PUBLIC_HEADERS}"   # 指定公共头文件
)

# ============================================================================
# 安装规则
# ============================================================================
# 安装库文件
install(TARGETS log_lib
    EXPORT LogLibTargets                # 导出目标
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # 库文件安装目录
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # 静态库安装目录
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # 可执行文件安装目录（Windows）
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log_lib  # 头文件安装目录
)

# 导出 cmake 配置文件
# install(EXPORT LogLibTargets
#     FILE LogLibTargets.cmake
#     NAMESPACE log_lib::                 # 命名空间前缀
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/log_lib
# )

# # 生成包配置文件
# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LogLibConfig.cmake.in
#     ${CMAKE_CURRENT_BINARY_DIR}/LogLibConfig.cmake
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/log_lib
# )

# # 生成版本配置文件
# write_basic_package_version_file(
#     ${CMAKE_CURRENT_BINARY_DIR}/LogLibConfigVersion.cmake
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion      # 主版本号相同即兼容
# )

# ============================================================================
# 子目录（条件编译）
# ============================================================================
if(BUILD_EXAMPLES)
    add_subdirectory(examples)          # 示例程序
endif()

if(BUILD_TESTS)
    enable_testing()                    # 启用测试
    add_subdirectory(tests)             # 单元测试
endif()

# ============================================================================
# 配置信息输出
# ============================================================================
message(STATUS "========================================")
message(STATUS "LogLib Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared library: ${BUILD_SHARED_LIBS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")

