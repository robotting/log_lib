#include "log_lib/logger.h"

namespace log_lib {

bool init(const Config& config) {
    return LoggerManager::getInstance().initWithSpdlog(config);
}

bool initDefault() {
    Config cfg;
    cfg.level = Level::INFO;
    cfg.file_path = "logs/app.log";
    cfg.max_file_size = 10 * 1024 * 1024;
    cfg.max_files = 5;
    cfg.enable_console = true;
    cfg.async_mode = true;
    return init(cfg);
}

void shutdown() {
    LoggerManager::getInstance().shutdown();
}

ILogger* getLogger() {
    return LoggerManager::getInstance().getLogger();
}

void setLevel(Level level) {
    if (auto* logger = getLogger()) {
        logger->setLevel(level);
    }
}

Level getLevel() {
    auto* logger = getLogger();
    return logger ? logger->getLevel() : Level::OFF;
}

void flush() {
    if (auto* logger = getLogger()) {
        logger->flush();
    }
}

bool shouldLog(Level level) {
    auto* logger = getLogger();
    return logger && logger->shouldLog(level);
}

} // namespace log_lib
