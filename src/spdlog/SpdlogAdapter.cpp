/**
 * @file SpdlogAdapter.cpp
 * @brief spdlog 适配器实现
 */

#include "log_lib/spdlog/SpdlogAdapter.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <cstdio>
#include <filesystem>

namespace log_lib {

namespace {

spdlog::level::level_enum toSpdlogLevel(Level level) {
    return static_cast<spdlog::level::level_enum>(level);
}

Level fromSpdlogLevel(spdlog::level::level_enum level) {
    return static_cast<Level>(level);
}

bool ensureLogDirectory(const std::string& file_path) {
    const auto parent = std::filesystem::path(file_path).parent_path();
    if (parent.empty()) {
        return true;
    }
    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec) {
        fprintf(stderr, "[SpdlogAdapter] Failed to create log directory '%s': %s\n",
                parent.string().c_str(), ec.message().c_str());
        return false;
    }
    return true;
}

} // namespace

SpdlogAdapter::~SpdlogAdapter() {
    shutdown();
}

bool SpdlogAdapter::init(const Config& config) {
    try {
        config_ = config;

        if (!ensureLogDirectory(config.file_path)) {
            return false;
        }

        if (config.async_mode) {
            spdlog::init_thread_pool(config.async_queue_size, 1);
        }

        std::vector<spdlog::sink_ptr> sinks;

        if (config.enable_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(config.pattern);
            if (!config.enable_color) {
                console_sink->set_color_mode(spdlog::color_mode::never);
            }
            sinks.push_back(console_sink);
        }

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.file_path,
            config.max_file_size,
            config.max_files
        );
        file_sink->set_pattern(config.pattern);
        sinks.push_back(file_sink);

        if (sinks.empty()) {
            fprintf(stderr, "[SpdlogAdapter] Init failed: no sinks configured\n");
            return false;
        }

        if (config.async_mode) {
            logger_ = std::make_shared<spdlog::async_logger>(
                "log_lib",
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        } else {
            logger_ = std::make_shared<spdlog::logger>(
                "log_lib", sinks.begin(), sinks.end()
            );
        }

        logger_->set_level(toSpdlogLevel(config.level));

        if (config.flush_on_error) {
            logger_->flush_on(spdlog::level::err);
        }

        if (config.flush_interval_seconds > 0) {
            spdlog::flush_every(std::chrono::seconds(config.flush_interval_seconds));
        }

        return true;
    } catch (const spdlog::spdlog_ex& ex) {
        fprintf(stderr, "[SpdlogAdapter] Init failed: %s\n", ex.what());
        return false;
    }
}

void SpdlogAdapter::shutdown() {
    if (logger_) {
        logger_->flush();
        logger_.reset();
    }
}

void SpdlogAdapter::writeLog(spdlog::level::level_enum spd_level, Level level,
                             const char* file, int line, const char* format, va_list args) {
    if (!logger_ || !shouldLog(level)) {
        return;
    }

    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);

    if (config_.include_source_info && file) {
        logger_->log(spdlog::source_loc{file, line, nullptr}, spd_level, "{}", buffer);
    } else {
        logger_->log(spdlog::source_loc{}, spd_level, "{}", buffer);
    }
}

void SpdlogAdapter::log(Level level, const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(toSpdlogLevel(level), level, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::trace(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::trace, Level::TRACE, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::debug(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::debug, Level::DEBUG, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::info(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::info, Level::INFO, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::warn(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::warn, Level::WARN, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::error(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::err, Level::ERROR, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::fatal(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeLog(spdlog::level::critical, Level::FATAL, file, line, format, args);
    va_end(args);
}

void SpdlogAdapter::setLevel(Level level) {
    if (logger_) {
        logger_->set_level(toSpdlogLevel(level));
    }
}

Level SpdlogAdapter::getLevel() const {
    if (logger_) {
        return fromSpdlogLevel(logger_->level());
    }
    return Level::OFF;
}

void SpdlogAdapter::flush() {
    if (logger_) {
        logger_->flush();
    }
}

bool SpdlogAdapter::shouldLog(Level level) const {
    if (!logger_) {
        return false;
    }
    return static_cast<int>(level) >= static_cast<int>(getLevel());
}

std::unique_ptr<ILogger> SpdlogFactory::create() {
    return std::make_unique<SpdlogAdapter>();
}

} // namespace log_lib
