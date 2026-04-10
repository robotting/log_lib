/**
 * @file logger.h
 * @brief 日志库统一入口头文件
 * 
 * 使用本日志库时，只需包含此头文件即可。
 * 本文件提供了最简洁的 API 接口。
 * 
 * @code
 * #include <log_lib/logger.h>
 * 
 * int main() {
 *     // 初始化
 *     log_lib::initDefault();
 *     
 *     // 记录日志
 *     LOG_INFO("Hello, {}!", "world");
 *     LOG_ERROR("Something went wrong: {}", "timeout");
 *     
 *     // 清理
 *     log_lib::shutdown();
 * }
 * @endcode
 */

#pragma once

// 包含所有必要的头文件
#include "ILogger.h"          // 接口定义
#include "LoggerManager.h"    // 管理器
#include "LoggerMacros.h"     // 宏定义
#include "spdlog/SpdlogAdapter.h"  // spdlog 适配器

namespace log_lib {

/**
 * @brief 初始化日志系统（使用 spdlog）
 * @param config 配置参数（可选）
 * @return 初始化成功返回 true
 * 
 * @note 这是推荐的初始化方式，内部使用 spdlog 实现
 */
inline bool init(const Config& config = Config()) {
    return LoggerManager::getInstance().initWithSpdlog(config);
}

/**
 * @brief 使用默认配置初始化
 * 
 * 默认配置：
 * - 日志级别：INFO
 * - 日志文件：logs/app.log
 * - 单文件大小：10MB
 * - 保留文件数：5
 * - 启用控制台输出
 * - 启用异步模式
 * 
 * @return 初始化成功返回 true
 */
inline bool initDefault() {
    Config cfg;
    cfg.level = Level::INFO;
    cfg.file_path = "logs/app.log";
    cfg.max_file_size = 10 * 1024 * 1024;
    cfg.max_files = 5;
    cfg.enable_console = true;
    cfg.async_mode = true;
    return init(cfg);
}

/**
 * @brief 关闭日志系统
 * 
 * 刷新所有缓冲区并释放资源。
 * 程序退出前应调用此函数。
 */
inline void shutdown() {
    LoggerManager::getInstance().shutdown();
}

/**
 * @brief 获取日志器实例
 * @return 日志器指针，未初始化时返回 nullptr
 * 
 * @note 通常不需要直接使用，宏已经封装好了
 */
inline ILogger* getLogger() {
    return LoggerManager::getInstance().getLogger();
}

/**
 * @brief 动态设置日志级别
 * @param level 新的日志级别
 * 
 * @code
 * log_lib::setLevel(log_lib::Level::DEBUG);  // 开启调试日志
 * log_lib::setLevel(log_lib::Level::WARN);   // 只记录警告及以上
 * @endcode
 */
inline void setLevel(Level level) {
    auto* logger = getLogger();
    if (logger) logger->setLevel(level);
}

/**
 * @brief 获取当前日志级别
 */
inline Level getLevel() {
    auto* logger = getLogger();
    return logger ? logger->getLevel() : Level::OFF;
}

/**
 * @brief 刷新日志缓冲区
 * 
 * 强制将所有缓冲的日志写入磁盘。
 * 通常在程序退出前或关键检查点调用。
 */
inline void flush() {
    auto* logger = getLogger();
    if (logger) logger->flush();
}

/**
 * @brief 检查指定级别是否会被记录
 * @param level 要检查的级别
 * @return 如果该级别会被记录返回 true
 * 
 * @note 可用于优化性能，避免昂贵的字符串构造
 */
inline bool shouldLog(Level level) {
    auto* logger = getLogger();
    return logger ? logger->shouldLog(level) : false;
}

} // namespace log_lib