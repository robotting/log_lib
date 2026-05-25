/**
 * @file SpdlogAdapter.h
 * @brief spdlog 适配器实现
 * 
 * 本文件实现了 ILogger 接口，将 spdlog 封装为统一的日志接口。
 * 这样业务代码可以完全不依赖 spdlog 的具体 API。
 */

#pragma once

#include "../Export.h"
#include "../ILogger.h"
#include <cstdarg>
#include <memory>
#include <spdlog/spdlog.h>

namespace log_lib {

/**
 * @brief spdlog 适配器类
 * 
 * 将 spdlog 的 API 适配到 ILogger 接口。
 * 
 * @note 这是 ILogger 的一个具体实现，如果需要切换到其他日志库，
 *       只需实现新的适配器类即可。
 */
class LOG_LIB_API SpdlogAdapter : public ILogger {
public:
    /**
     * @brief 构造函数
     */
    SpdlogAdapter() = default;
    
    /**
     * @brief 析构函数
     * 
     * 自动调用 shutdown() 确保资源释放
     */
    ~SpdlogAdapter() override;
    
    // ========== ILogger 接口实现 ==========
    
    /**
     * @brief 初始化 spdlog
     * 
     * 实现逻辑：
     * 1. 根据配置决定是否启用异步模式
     * 2. 创建控制台 sink（如果启用）
     * 3. 创建文件 sink（自动轮转）
     * 4. 组合 sinks 创建 logger
     * 5. 设置日志级别和刷新策略
     */
    bool init(const Config& config) override;
    
    /**
     * @brief 关闭 spdlog
     * 
     * 刷新所有缓冲区并释放资源
     */
    void shutdown() override;
    
    /**
     * @brief 通用日志记录
     * @param level 日志级别
     * @param file 源文件名
     * @param line 源文件行号
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    void log(Level level, const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 TRACE 级别日志
     */
    void trace(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 DEBUG 级别日志
     */
    void debug(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 INFO 级别日志
     */
    void info(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 WARN 级别日志
     */
    void warn(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 ERROR 级别日志
     */
    void error(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 记录 FATAL 级别日志
     */
    void fatal(const char* file, int line, const char* format, ...) override;
    
    /**
     * @brief 设置日志级别
     */
    void setLevel(Level level) override;
    
    /**
     * @brief 获取当前日志级别
     */
    Level getLevel() const override;
    
    /**
     * @brief 刷新缓冲区
     */
    void flush() override;
    
    /**
     * @brief 检查是否应该记录指定级别
     */
    bool shouldLog(Level level) const override;
    
private:
    void writeLog(spdlog::level::level_enum spd_level, Level level,
                  const char* file, int line, const char* format, va_list args);

    Config config_;                              ///< 配置信息
    std::shared_ptr<spdlog::logger> logger_;    ///< spdlog 日志器实例
};

/**
 * @brief spdlog 工厂类
 * 
 * 用于创建 SpdlogAdapter 实例
 */
class LOG_LIB_API SpdlogFactory : public ILoggerFactory {
public:
    /**
     * @brief 创建 spdlog 适配器实例
     * @return 新创建的适配器实例
     */
    std::unique_ptr<ILogger> create() override;
};

} // namespace log_lib