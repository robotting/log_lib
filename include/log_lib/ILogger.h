/**
 * @file ILogger.h
 * @brief 日志系统抽象接口定义
 * 
 * 本文件定义了日志系统的核心抽象接口，包括：
 * - 日志级别枚举
 * - 配置结构体
 * - 日志器抽象基类
 * - 工厂抽象基类
 * 
 */

#pragma once

#include <string>
#include <memory>
#include <cstdarg>

namespace log_lib {

/**
 * @brief 日志级别枚举
 * 
 * 定义了从最详细到最严重的日志级别，
 * 数值越小表示级别越低（越详细），
 * 数值越大表示级别越高（越严重）。
 */
enum class Level {
    TRACE = 0,   ///< 最详细的跟踪信息，用于追踪程序执行流程
    DEBUG = 1,   ///< 调试信息，仅在开发/测试环境使用
    INFO  = 2,   ///< 常规信息，记录关键业务流程节点
    WARN  = 3,   ///< 警告信息，表示潜在问题但不影响正常运行
    ERROR = 4,   ///< 错误信息，表示某个操作失败但程序可继续运行
    FATAL = 5,   ///< 致命错误，程序即将退出
    OFF   = 6    ///< 关闭所有日志输出
};

/**
 * @brief 日志系统配置结构体
 * 
 * 包含了日志系统的所有配置选项，
 * 可以在初始化时传入，也可以在运行时动态修改部分选项。
 */
struct Config {
    // ========== 基础配置 ==========
    
    /** @brief 全局日志级别，低于此级别的日志不会被记录 */
    Level level = Level::INFO;
    
    /** 
     * @brief 日志输出格式
     * 
     * 支持的格式说明符：
     * %Y - 年份（4位）
     * %m - 月份（01-12）
     * %d - 日期（01-31）
     * %H - 小时（00-23）
     * %M - 分钟（00-59）
     * %S - 秒（00-60）
     * %e - 毫秒（000-999）
     * %l - 日志级别名称（小写）
     * %L - 日志级别名称（大写）
     * %^ - 开始颜色
     * %$ - 结束颜色
     * %t - 线程ID
     * %s - 源文件名
     * %# - 源文件行号
     * %v - 日志消息内容
     */
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%t] %v";
    
    // ========== 文件输出配置 ==========
    
    /** @brief 日志文件路径 */
    std::string file_path = "logs/app.log";
    
    /** @brief 单个日志文件最大大小（字节），超过后自动轮转 */
    size_t max_file_size = 10 * 1024 * 1024;  // 默认 10MB
    
    /** @brief 最多保留的日志文件数量 */
    int max_files = 5;
    
    /** @brief 是否同时输出到控制台 */
    bool enable_console = true;
    
    // ========== 异步模式配置 ==========
    
    /** @brief 是否启用异步日志模式（推荐生产环境开启） */
    bool async_mode = true;
    
    /** @brief 异步日志队列大小，队列满时的行为由 overflow_policy 决定 */
    size_t async_queue_size = 8192;
    
    // ========== 刷新策略配置 ==========
    
    /** @brief 是否在每次 ERROR 级别日志后自动刷新 */
    bool flush_on_error = true;
    
    /** @brief 定时刷新间隔（秒），0 表示不自动刷新 */
    int flush_interval_seconds = 5;
    
    // ========== 高级配置 ==========
    
    /** @brief 是否在日志中包含源文件名和行号 */
    bool include_source_info = true;
    
    /** @brief 是否启用颜色输出（仅控制台） */
    bool enable_color = true;
};

/**
 * @brief 日志器抽象接口
 * 
 * 定义了所有日志器实现必须提供的方法。
 * 这是典型的"依赖倒置"原则：业务代码依赖抽象接口，而不是具体实现。
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    
    // ========== 生命周期管理 ==========
    
    /**
     * @brief 初始化日志器
     * @param config 配置参数
     * @return 初始化成功返回 true，失败返回 false
     */
    virtual bool init(const Config& config) = 0;
    
    /**
     * @brief 关闭日志器，刷新所有缓冲区
     */
    virtual void shutdown() = 0;
    
    // ========== 核心日志记录接口 ==========
    
    /**
     * @brief 通用日志记录方法（可变参数版本）
     * @param level 日志级别
     * @param file 源文件名（通常使用 __FILE__ 宏）
     * @param line 源文件行号（通常使用 __LINE__ 宏）
     * @param format printf 风格的格式化字符串
     * @param ... 格式化参数
     * 
     * @note 使用示例：logger->log(Level::INFO, __FILE__, __LINE__, "Value: %d", 42);
     */
    virtual void log(Level level, const char* file, int line, const char* format, ...) = 0;
    
    // ========== 便捷日志记录接口（按级别） ==========
    
    /**
     * @brief 记录 TRACE 级别日志
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 格式化参数
     */
    virtual void trace(const char* file, int line, const char* format, ...) = 0;
    
    /**
     * @brief 记录 DEBUG 级别日志
     */
    virtual void debug(const char* file, int line, const char* format, ...) = 0;
    
    /**
     * @brief 记录 INFO 级别日志
     */
    virtual void info(const char* file, int line, const char* format, ...) = 0;
    
    /**
     * @brief 记录 WARN 级别日志
     */
    virtual void warn(const char* file, int line, const char* format, ...) = 0;
    
    /**
     * @brief 记录 ERROR 级别日志
     */
    virtual void error(const char* file, int line, const char* format, ...) = 0;
    
    /**
     * @brief 记录 FATAL 级别日志
     * @note 记录后通常会触发程序退出
     */
    virtual void fatal(const char* file, int line, const char* format, ...) = 0;
    
    // ========== 运行时控制接口 ==========
    
    /**
     * @brief 动态设置日志级别
     * @param level 新的日志级别
     * 
     * @note 可以在程序运行时调整，无需重启
     */
    virtual void setLevel(Level level) = 0;
    
    /**
     * @brief 获取当前日志级别
     * @return 当前日志级别
     */
    virtual Level getLevel() const = 0;
    
    /**
     * @brief 强制刷新所有日志缓冲区
     * 
     * @note 确保所有待写入的日志都被写入磁盘
     */
    virtual void flush() = 0;
    
    /**
     * @brief 检查指定级别是否会被记录
     * @param level 要检查的日志级别
     * @return 如果该级别会被记录返回 true
     * 
     * @note 可用于避免不必要的字符串构造开销
     * @code
     * if (logger->shouldLog(Level::DEBUG)) {
     *     logger->debug(__FILE__, __LINE__, "Expensive: %s", expensiveToString().c_str());
     * }
     * @endcode
     */
    virtual bool shouldLog(Level level) const = 0;
};

/**
 * @brief 日志器工厂抽象接口
 * 
 * 使用工厂模式创建具体的日志器实例，
 * 便于在不同实现之间切换。
 */
class ILoggerFactory {
public:
    virtual ~ILoggerFactory() = default;
    
    /**
     * @brief 创建日志器实例
     * @return 日志器实例的 unique_ptr
     */
    virtual std::unique_ptr<ILogger> create() = 0;
};

/**
 * @brief 辅助函数：将级别转换为字符串
 * @param level 日志级别
 * @return 对应的字符串表示
 */
inline const char* levelToString(Level level) {
    switch (level) {
        case Level::TRACE: return "TRACE";
        case Level::DEBUG: return "DEBUG";
        case Level::INFO:  return "INFO";
        case Level::WARN:  return "WARN";
        case Level::ERROR: return "ERROR";
        case Level::FATAL: return "FATAL";
        case Level::OFF:   return "OFF";
        default:           return "UNKNOWN";
    }
}

/**
 * @brief 辅助函数：从字符串解析日志级别
 * @param str 字符串表示
 * @return 对应的日志级别，无法解析时返回 Level::INFO
 */
inline Level stringToLevel(const std::string& str) {
    if (str == "TRACE" || str == "trace") return Level::TRACE;
    if (str == "DEBUG" || str == "debug") return Level::DEBUG;
    if (str == "INFO"  || str == "info")  return Level::INFO;
    if (str == "WARN"  || str == "warn")  return Level::WARN;
    if (str == "ERROR" || str == "error") return Level::ERROR;
    if (str == "FATAL" || str == "fatal") return Level::FATAL;
    if (str == "OFF"   || str == "off")   return Level::OFF;
    return Level::INFO;  // 默认返回 INFO
}

} // namespace log_lib