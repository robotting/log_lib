/**
 * @file LoggerManager.cpp
 * @brief 日志管理器实现
 */

#include "log_lib/LoggerManager.h"
#include "log_lib/spdlog/SpdlogAdapter.h"
#include <iostream>

namespace log_lib {

// ============================================================================
// 单例实现
// ============================================================================

/**
 * @brief 获取单例实例
 * 
 * 使用 Meyers 单例模式：
 * - C++11 保证静态局部变量的初始化是线程安全的
 * - 第一次调用时构造，程序结束时自动析构
 */
LoggerManager& LoggerManager::getInstance() {
    static LoggerManager instance;
    return instance;
}

// ============================================================================
// 构造与析构
// ============================================================================

/**
 * @brief 构造函数
 * 
 * 初始化为未初始化状态
 */
LoggerManager::LoggerManager() 
    : initialized_(false) {
}

/**
 * @brief 析构函数
 * 
 * 自动关闭日志系统，释放资源
 */
LoggerManager::~LoggerManager() {
    shutdown();
}

// ============================================================================
// 初始化与关闭
// ============================================================================

/**
 * @brief 初始化日志系统
 * 
 * 实现逻辑：
 * 1. 如果已经初始化，先关闭现有实例
 * 2. 使用工厂创建具体的日志器实例
 * 3. 调用日志器的 init 方法
 * 4. 记录初始化成功日志
 */
bool LoggerManager::init(std::unique_ptr<ILoggerFactory> factory, const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 如果已经初始化，先关闭
    if (initialized_) {
        std::cerr << "[LogLib] Warning: Already initialized, shutting down first..." << std::endl;
        shutdown();
    }
    
    // 保存工厂和配置
    factory_ = std::move(factory);
    logger_ = factory_->create();
    
    // 初始化日志器
    if (!logger_->init(config)) {
        std::cerr << "[LogLib] Error: Failed to initialize logger" << std::endl;
        return false;
    }
    
    // 保存配置
    config_ = config;
    initialized_ = true;
    
    // 记录初始化成功日志（此时日志器已经可用）
    if (logger_) {
        logger_->info(__FILE__, __LINE__, "LogLib initialized successfully");
    }
    
    return true;
}

/**
 * @brief 使用 spdlog 初始化
 * 
 * 这是最便捷的初始化方式，内部创建 SpdlogFactory
 */
bool LoggerManager::initWithSpdlog(const Config& config) {
    return init(std::make_unique<SpdlogFactory>(), config);
}

/**
 * @brief 关闭日志系统
 * 
 * 实现逻辑：
 * 1. 记录关闭日志
 * 2. 调用日志器的 shutdown 方法
 * 3. 释放资源
 */
void LoggerManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (logger_) {
        // 记录关闭日志
        logger_->info(__FILE__, __LINE__, "LogLib shutting down...");
        
        // 关闭日志器
        logger_->shutdown();
        
        // 释放资源
        logger_.reset();
        factory_.reset();
    }
    
    initialized_ = false;
}

// ============================================================================
// 公共接口
// ============================================================================

/**
 * @brief 获取日志器实例
 * 
 * @return 日志器指针，未初始化时返回 nullptr
 * 
 * @note 使用前应先检查返回值是否为空
 */
ILogger* LoggerManager::getLogger() {
    std::lock_guard<std::mutex> lock(mutex_);
    return logger_.get();
}

/**
 * @brief 检查是否已初始化
 */
bool LoggerManager::isInitialized() const {
    return initialized_;
}

/**
 * @brief 获取当前配置
 * 
 * @return 配置的副本
 */
Config LoggerManager::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

} // namespace log_lib