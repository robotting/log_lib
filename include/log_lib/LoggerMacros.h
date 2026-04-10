/**
 * @file LoggerMacros.h
 * @brief 日志宏定义
 * 
 * 本文件提供了便捷的日志宏，简化日志记录代码。
 * 
 * 使用示例：
 * @code
 * LOG_INFO("User {} logged in", username);
 * LOG_ERROR("Database error: {}", error_msg);
 * LOG_INFO_IF(retry_count > 0, "Retry attempt {}", retry_count);
 * LOG_INFO_EVERY_N(10, "Processing batch {}", batch_id);
 * @endcode
 */

#pragma once

#include "LoggerManager.h"

/**
 * @brief 获取日志器实例的宏
 * 
 * 内部使用，自动处理空指针情况。
 */
#define LOG_GET_LOGGER() ::log_lib::LoggerManager::getInstance().getLogger()

/**
 * @brief 条件日志宏（内部使用）
 * 
 * 检查日志器是否有效以及级别是否启用，
 * 如果满足条件则记录日志。
 */
#define LOG_IF_ENABLED(level, ...) \
    do { \
        auto* _log = LOG_GET_LOGGER(); \
        if (_log && _log->shouldLog(::log_lib::Level::level)) { \
            _log->level(__FILE__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

// ============================================================================
// 基础日志宏（按级别）
// ============================================================================

/**
 * @brief 记录 TRACE 级别日志
 * @param ... 格式化字符串和参数（printf 风格）
 * 
 * @note TRACE 级别最详细，用于追踪程序执行流程
 */
#define LOG_TRACE(...) LOG_IF_ENABLED(trace, __VA_ARGS__)

/**
 * @brief 记录 DEBUG 级别日志
 * @param ... 格式化字符串和参数
 * 
 * @note DEBUG 级别用于调试信息，生产环境通常关闭
 */
#define LOG_DEBUG(...) LOG_IF_ENABLED(debug, __VA_ARGS__)

/**
 * @brief 记录 INFO 级别日志
 * @param ... 格式化字符串和参数
 * 
 * @note INFO 级别用于记录关键业务流程
 */
#define LOG_INFO(...)  LOG_IF_ENABLED(info, __VA_ARGS__)

/**
 * @brief 记录 WARN 级别日志
 * @param ... 格式化字符串和参数
 * 
 * @note WARN 级别用于警告潜在问题
 */
#define LOG_WARN(...)  LOG_IF_ENABLED(warn, __VA_ARGS__)

/**
 * @brief 记录 ERROR 级别日志
 * @param ... 格式化字符串和参数
 * 
 * @note ERROR 级别用于记录错误，需要关注
 */
#define LOG_ERROR(...) LOG_IF_ENABLED(error, __VA_ARGS__)

/**
 * @brief 记录 FATAL 级别日志
 * @param ... 格式化字符串和参数
 * 
 * @note FATAL 级别用于致命错误，记录后程序通常会退出
 */
#define LOG_FATAL(...) LOG_IF_ENABLED(fatal, __VA_ARGS__)

// ============================================================================
// 条件日志宏
// ============================================================================

/**
 * @brief 条件满足时记录 INFO 日志
 * @param cond 条件表达式
 * @param ... 格式化字符串和参数
 * 
 * @code
 * LOG_INFO_IF(error_count > 0, "Found {} errors", error_count);
 * @endcode
 */
#define LOG_INFO_IF(cond, ...) if (cond) LOG_INFO(__VA_ARGS__)

/**
 * @brief 条件满足时记录 DEBUG 日志
 */
#define LOG_DEBUG_IF(cond, ...) if (cond) LOG_DEBUG(__VA_ARGS__)

/**
 * @brief 条件满足时记录 WARN 日志
 */
#define LOG_WARN_IF(cond, ...) if (cond) LOG_WARN(__VA_ARGS__)

/**
 * @brief 条件满足时记录 ERROR 日志
 */
#define LOG_ERROR_IF(cond, ...) if (cond) LOG_ERROR(__VA_ARGS__)

// ============================================================================
// 频率限制日志宏
// ============================================================================

/**
 * @brief 每 N 次调用记录一次 INFO 日志
 * @param n 间隔次数
 * @param ... 格式化字符串和参数
 * 
 * @note 用于避免日志洪水
 * @code
 * for (int i = 0; i < 1000; i++) {
 *     LOG_INFO_EVERY_N(100, "Processing iteration {}", i);  // 每100次记录一次
 * }
 * @endcode
 */
#define LOG_INFO_EVERY_N(n, ...) \
    do { \
        static int _count = 0; \
        if (++_count % (n) == 0) { \
            LOG_INFO(__VA_ARGS__); \
        } \
    } while(0)

/**
 * @brief 每 N 次调用记录一次 DEBUG 日志
 */
#define LOG_DEBUG_EVERY_N(n, ...) \
    do { \
        static int _count = 0; \
        if (++_count % (n) == 0) { \
            LOG_DEBUG(__VA_ARGS__); \
        } \
    } while(0)

/**
 * @brief 每 N 次调用记录一次 WARN 日志
 */
#define LOG_WARN_EVERY_N(n, ...) \
    do { \
        static int _count = 0; \
        if (++_count % (n) == 0) { \
            LOG_WARN(__VA_ARGS__); \
        } \
    } while(0)

// ============================================================================
// 首次/前N次日志宏
// ============================================================================

/**
 * @brief 前 N 次调用记录 INFO 日志
 * @param n 前几次
 * @param ... 格式化字符串和参数
 * 
 * @code
 * LOG_INFO_FIRST_N(3, "First few calls: {}", call_count);
 * @endcode
 */
#define LOG_INFO_FIRST_N(n, ...) \
    do { \
        static int _count = 0; \
        if (_count++ < (n)) { \
            LOG_INFO(__VA_ARGS__); \
        } \
    } while(0)

/**
 * @brief 前 N 次调用记录 DEBUG 日志
 */
#define LOG_DEBUG_FIRST_N(n, ...) \
    do { \
        static int _count = 0; \
        if (_count++ < (n)) { \
            LOG_DEBUG(__VA_ARGS__); \
        } \
    } while(0)

/**
 * @brief 前 N 次调用记录 WARN 日志
 */
#define LOG_WARN_FIRST_N(n, ...) \
    do { \
        static int _count = 0; \
        if (_count++ < (n)) { \
            LOG_WARN(__VA_ARGS__); \
        } \
    } while(0)

// ============================================================================
// 简写宏（可选）
// ============================================================================

/**
 * @brief 简写版本的日志宏
 * 
 * 如果觉得 LOG_INFO 太长，可以使用以下简写：
 * - LOGI  -> LOG_INFO
 * - LOGW  -> LOG_WARN
 * - LOGE  -> LOG_ERROR
 * - LOGD  -> LOG_DEBUG
 * - LOGF  -> LOG_FATAL
 */
#define LOGI(...)  LOG_INFO(__VA_ARGS__)
#define LOGW(...)  LOG_WARN(__VA_ARGS__)
#define LOGE(...)  LOG_ERROR(__VA_ARGS__)
#define LOGD(...)  LOG_DEBUG(__VA_ARGS__)
#define LOGF(...)  LOG_FATAL(__VA_ARGS__)