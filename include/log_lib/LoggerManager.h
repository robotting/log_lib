/**
 * @file LoggerManager.h
 * @brief 日志管理器（单例模式）
 * 
 * 本文件定义了日志管理器的实现，采用单例模式确保全局只有一个日志器实例。
 * 主要职责：
 * - 管理日志器的生命周期
 * - 提供统一的访问入口
 * - 支持动态切换日志实现
 */

#pragma once

#include "Export.h"
#include "ILogger.h"
#include <memory>
#include <mutex>

namespace log_lib {

/**
 * @brief 日志管理器类（单例模式）
 * 
 * 这是日志系统的核心管理类，负责：
 * 1. 创建和销毁日志器实例
 * 2. 提供全局访问点
 * 3. 线程安全的初始化和访问
 * 
 * @note 该类为线程安全的单例模式
 * 
 * @code
 * // 使用示例
 * Config config;
 * config.level = Level::INFO;
 * LoggerManager::getInstance().init(std::make_unique<SpdlogFactory>(), config);
 * 
 * auto* logger = LoggerManager::getInstance().getLogger();
 * logger->info(__FILE__, __LINE__, "Hello");
 * @endcode
 */
class LOG_LIB_API LoggerManager {
public:
    /**
     * @brief 获取单例实例
     * @return 日志管理器单例引用
     * 
     * @note 使用 Meyers 单例模式，线程安全且自动初始化
     */
    static LoggerManager& getInstance();
    
    /**
     * @brief 初始化日志系统
     * @param factory 日志器工厂（用于创建具体的日志实现）
     * @param config 配置参数
     * @return 初始化成功返回 true，失败返回 false
     * 
     * @note 如果已经初始化，会先调用 shutdown() 再重新初始化
     * @note 该方法线程安全
     */
    bool init(std::unique_ptr<ILoggerFactory> factory, const Config& config = Config());

    /**
     * @brief 使用 spdlog 实现初始化日志系统
     * @param config 配置参数
     * @return 初始化成功返回 true
     */
    bool initWithSpdlog(const Config& config = Config());
    
    /**
     * @brief 关闭日志系统
     * 
     * 刷新所有缓冲区并释放资源。
     * 关闭后可以再次调用 init() 重新初始化。
     * 
     * @note 该方法线程安全
     */
    void shutdown();
    
    /**
     * @brief 获取日志器实例
     * @return 日志器指针，未初始化时返回 nullptr
     * 
     * @note 该方法线程安全
     */
    ILogger* getLogger();
    
    /**
     * @brief 检查日志系统是否已初始化
     * @return 已初始化返回 true
     */
    bool isInitialized() const;
    
    /**
     * @brief 获取当前配置
     * @return 当前使用的配置副本
     */
    Config getConfig() const;
    
    // ========== 禁止拷贝和移动 ==========
    LoggerManager(const LoggerManager&) = delete;            ///< 禁止拷贝构造
    LoggerManager& operator=(const LoggerManager&) = delete; ///< 禁止拷贝赋值
    LoggerManager(LoggerManager&&) = delete;                 ///< 禁止移动构造
    LoggerManager& operator=(LoggerManager&&) = delete;      ///< 禁止移动赋值
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    LoggerManager();
    
    /**
     * @brief 析构函数（自动调用 shutdown）
     */
    ~LoggerManager();

    /** @brief 在已持有 mutex_ 时关闭（避免重入死锁） */
    void shutdownUnlocked();
    
    mutable std::mutex mutex_;              ///< 互斥锁，保证线程安全
    bool initialized_;                      ///< 初始化标志
    Config config_;                         ///< 当前配置
    std::unique_ptr<ILoggerFactory> factory_; ///< 日志器工厂
    std::unique_ptr<ILogger> logger_;       ///< 日志器实例
};

} // namespace log_lib