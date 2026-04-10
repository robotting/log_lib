/**
 * @file SpdlogAdapter.cpp
 * @brief spdlog 适配器实现
 */

#include "log_lib/spdlog/SpdlogAdapter.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <cstdarg>
#include <cstdio>

namespace log_lib {

    // ============================================================================
    // 辅助函数：级别转换
    // ============================================================================

    /**
     * @brief 将内部日志级别转换为 spdlog 日志级别
     * @param level 内部日志级别
     * @return spdlog 日志级别
     */
    static spdlog::level::level_enum toSpdlogLevel(Level level) {
        return static_cast<spdlog::level::level_enum>(level);
    }

    /**
     * @brief 将 spdlog 日志级别转换为内部日志级别
     * @param level spdlog 日志级别
     * @return 内部日志级别
     */
    static Level fromSpdlogLevel(spdlog::level::level_enum level) {
        return static_cast<Level>(level);
    }

    // ============================================================================
    // 构造与析构
    // ============================================================================

    /**
     * @brief 析构函数
     *
     * 确保资源正确释放
     */
    SpdlogAdapter::~SpdlogAdapter() {
        shutdown();
    }

    // ============================================================================
    // 初始化与关闭
    // ============================================================================

    /**
     * @brief 初始化 spdlog
     *
     * 详细的初始化步骤：
     * 1. 保存配置
     * 2. 如果启用异步模式，初始化线程池
     * 3. 创建控制台 sink（如果启用）
     * 4. 创建文件 sink（支持自动轮转）
     * 5. 组合 sinks 创建 logger
     * 6. 设置日志级别
     * 7. 设置刷新策略
     */
    bool SpdlogAdapter::init(const Config& config) {
        try {
            // 保存配置
            config_ = config;

            // 1. 异步模式初始化
            if (config.async_mode) {
                spdlog::init_thread_pool(config.async_queue_size, 1);
            }

            // 2. 创建 sinks 列表
            std::vector<spdlog::sink_ptr> sinks;

            // 2.1 控制台 sink（如果启用）
            if (config.enable_console) {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_pattern(config.pattern);
                sinks.push_back(console_sink);
            }

            // 2.2 文件 sink（支持自动轮转）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config.file_path,           // 文件路径
                config.max_file_size,       // 单个文件最大大小
                config.max_files            // 最多保留文件数
            );
            file_sink->set_pattern(config.pattern);
            sinks.push_back(file_sink);

            // 3. 创建 logger
            if (config.async_mode) {
                // 异步 logger
                logger_ = std::make_shared<spdlog::async_logger>(
                    "log_lib",                           // logger 名称
                    sinks.begin(),                       // sinks 起始迭代器
                    sinks.end(),                         // sinks 结束迭代器
                    spdlog::thread_pool(),               // 线程池
                    spdlog::async_overflow_policy::block // 队列满时阻塞
                );
            }
            else {
                // 同步 logger
                logger_ = std::make_shared<spdlog::logger>(
                    "log_lib", sinks.begin(), sinks.end()
                );
            }

            // 4. 设置日志级别
            logger_->set_level(toSpdlogLevel(config.level));

            // 5. 设置刷新策略
            if (config.flush_on_error) {
                logger_->flush_on(spdlog::level::err);  // ERROR 级别自动刷新
            }

            if (config.flush_interval_seconds > 0) {
                spdlog::flush_every(std::chrono::seconds(config.flush_interval_seconds));
            }

            return true;

        }
        catch (const spdlog::spdlog_ex& ex) {
            // 初始化失败，输出错误信息到 stderr
            fprintf(stderr, "[SpdlogAdapter] Init failed: %s\n", ex.what());
            return false;
        }
    }

    /**
     * @brief 关闭 spdlog
     *
     * 刷新所有缓冲区并释放资源
     */
    void SpdlogAdapter::shutdown() {
        if (logger_) {
            logger_->flush();   // 刷新所有待写入的日志
            logger_.reset();    // 释放 logger 资源
        }
    }

    // ============================================================================
    // 核心日志方法
    // ============================================================================

    /**
     * @brief 通用日志记录方法
     *
     * 实现步骤：
     * 1. 检查 logger 是否有效
     * 2. 检查是否需要记录该级别
     * 3. 格式化消息（vsnprintf）
     * 4. 调用 spdlog 输出
     */
    void SpdlogAdapter::log(Level level, const char* file, int line, const char* format, ...) {
        // 检查有效性
        if (!logger_ || !shouldLog(level)) return;

        // 格式化消息
        va_list args;
        va_start(args, format);
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        // 输出日志
        logger_->log(toSpdlogLevel(level), buffer);
    }

    // ============================================================================
    // TRACE 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 TRACE 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * TRACE 是最详细的日志级别，用于追踪程序执行流程，
     * 包括函数进入/退出、变量值变化等细节信息。
     * 生产环境通常关闭此级别。
     */
    void SpdlogAdapter::trace(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 TRACE 级别是否启用
        if (!logger_ || !shouldLog(Level::TRACE)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息，使用 4KB 缓冲区
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 TRACE 级别日志
        logger_->log(spdlog::level::trace, buffer);
    }

    // ============================================================================
    // DEBUG 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 DEBUG 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * DEBUG 级别用于调试信息，通常在开发/测试环境开启，
     * 生产环境建议关闭以获得更好性能。
     */
    void SpdlogAdapter::debug(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 DEBUG 级别是否启用
        if (!logger_ || !shouldLog(Level::DEBUG)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 DEBUG 级别日志
        logger_->log(spdlog::level::debug, buffer);
    }

    // ============================================================================
    // INFO 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 INFO 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * INFO 级别用于记录关键业务流程节点，
     * 如用户登录、订单创建、服务启动/停止等。
     * 这是生产环境最常用的日志级别。
     */
    void SpdlogAdapter::info(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 INFO 级别是否启用
        if (!logger_ || !shouldLog(Level::INFO)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 INFO 级别日志
        logger_->log(spdlog::level::info, buffer);
    }

    // ============================================================================
    // WARN 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 WARN 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * WARN 级别用于警告潜在问题，
     * 如磁盘空间不足、重试操作、即将达到阈值等。
     * 警告不影响程序正常运行，但需要关注。
     */
    void SpdlogAdapter::warn(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 WARN 级别是否启用
        if (!logger_ || !shouldLog(Level::WARN)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 WARN 级别日志
        logger_->log(spdlog::level::warn, buffer);
    }

    // ============================================================================
    // ERROR 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 ERROR 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * ERROR 级别用于记录错误信息，
     * 如数据库连接失败、网络超时、文件读写错误等。
     * 错误通常需要运维人员介入处理。
     */
    void SpdlogAdapter::error(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 ERROR 级别是否启用
        if (!logger_ || !shouldLog(Level::ERROR)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 ERROR 级别日志
        logger_->log(spdlog::level::err, buffer);
    }

    // ============================================================================
    // FATAL 级别日志方法
    // ============================================================================

    /**
     * @brief 记录 FATAL 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     *
     * FATAL 级别用于致命错误，
     * 如无法恢复的系统错误、关键资源缺失等。
     * 记录 FATAL 日志后，程序通常会终止运行。
     */
    void SpdlogAdapter::fatal(const char* file, int line, const char* format, ...) {
        // 检查 logger 是否有效，以及 FATAL 级别是否启用
        if (!logger_ || !shouldLog(Level::FATAL)) return;

        // 使用可变参数列表
        va_list args;
        va_start(args, format);

        // 格式化日志消息
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 清理可变参数列表
        va_end(args);

        // 调用 spdlog 输出 FATAL 级别日志（使用 critical 级别）
        logger_->log(spdlog::level::critical, buffer);
    }

    // ============================================================================
    // 控制方法
    // ============================================================================

    /**
     * @brief 设置日志级别
     * @param level 新的日志级别
     *
     * 可以在程序运行时动态调整日志级别，无需重启。
     * 例如：调试时临时开启 DEBUG 级别，问题解决后恢复 INFO 级别。
     */
    void SpdlogAdapter::setLevel(Level level) {
        if (logger_) {
            logger_->set_level(toSpdlogLevel(level));
        }
    }

    /**
     * @brief 获取当前日志级别
     * @return 当前日志级别
     */
    Level SpdlogAdapter::getLevel() const {
        if (logger_) {
            return fromSpdlogLevel(logger_->level());
        }
        return Level::OFF;  // 未初始化时返回 OFF
    }

    /**
     * @brief 刷新缓冲区
     *
     * 强制将所有缓冲的日志写入磁盘。
     * 通常在程序退出前或关键检查点调用。
     */
    void SpdlogAdapter::flush() {
        if (logger_) {
            logger_->flush();
        }
    }

    /**
     * @brief 检查是否应该记录指定级别
     * @param level 要检查的日志级别
     * @return 如果该级别会被记录返回 true
     *
     * 这个检查可以避免不必要的字符串格式化开销：
     * @code
     * if (logger->shouldLog(Level::DEBUG)) {
     *     logger->debug(__FILE__, __LINE__, "Expensive: %s", expensiveToString().c_str());
     * }
     * @endcode
     */
    bool SpdlogAdapter::shouldLog(Level level) const {
        if (!logger_) return false;
        return level >= getLevel();
    }

    // ============================================================================
    // 工厂方法实现
    // ============================================================================

    /**
     * @brief 创建 spdlog 适配器实例
     * @return 新创建的适配器实例
     */
    std::unique_ptr<ILogger> SpdlogFactory::create() {
        return std::make_unique<SpdlogAdapter>();
    }

} // namespace log_lib