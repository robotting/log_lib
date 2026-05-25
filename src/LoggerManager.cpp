/**
 * @file LoggerManager.cpp
 * @brief 日志管理器实现
 */

#include "log_lib/LoggerManager.h"
#include "log_lib/spdlog/SpdlogAdapter.h"
#include <iostream>

namespace log_lib {

LoggerManager& LoggerManager::getInstance() {
    static LoggerManager instance;
    return instance;
}

LoggerManager::LoggerManager()
    : initialized_(false) {
}

LoggerManager::~LoggerManager() {
    shutdown();
}

bool LoggerManager::initWithSpdlog(const Config& config) {
    return init(std::make_unique<SpdlogFactory>(), config);
}

bool LoggerManager::init(std::unique_ptr<ILoggerFactory> factory, const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        std::cerr << "[LogLib] Warning: Already initialized, shutting down first..." << std::endl;
        shutdownUnlocked();
    }

    if (!factory) {
        std::cerr << "[LogLib] Error: Logger factory is null" << std::endl;
        return false;
    }

    factory_ = std::move(factory);
    logger_ = factory_->create();

    if (!logger_->init(config)) {
        std::cerr << "[LogLib] Error: Failed to initialize logger" << std::endl;
        logger_.reset();
        factory_.reset();
        initialized_ = false;
        return false;
    }

    config_ = config;
    initialized_ = true;

    if (logger_) {
        logger_->info(__FILE__, __LINE__, "LogLib initialized successfully");
    }

    return true;
}

void LoggerManager::shutdownUnlocked() {
    if (logger_) {
        logger_->info(__FILE__, __LINE__, "LogLib shutting down...");
        logger_->shutdown();
        logger_.reset();
        factory_.reset();
    }
    initialized_ = false;
}

void LoggerManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdownUnlocked();
}

ILogger* LoggerManager::getLogger() {
    std::lock_guard<std::mutex> lock(mutex_);
    return logger_.get();
}

bool LoggerManager::isInitialized() const {
    return initialized_;
}

Config LoggerManager::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

} // namespace log_lib
