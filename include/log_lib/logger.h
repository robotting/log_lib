/**
 * @file logger.h
 * @brief 日志库统一入口头文件
 *
 * 使用本日志库时，只需包含此头文件即可。
 *
 * @code
 * #include <log_lib/logger.h>
 *
 * int main() {
 *     log_lib::initDefault();
 *
 *     LOG_INFO("Hello, %s!", "world");
 *     LOG_ERROR("Something went wrong: %s", "timeout");
 *
 *     log_lib::shutdown();
 * }
 * @endcode
 */

#pragma once

#include "Export.h"
#include "ILogger.h"
#include "LoggerManager.h"
#include "LoggerMacros.h"

namespace log_lib {

LOG_LIB_API bool init(const Config& config = Config());

LOG_LIB_API bool initDefault();

LOG_LIB_API void shutdown();

LOG_LIB_API ILogger* getLogger();

LOG_LIB_API void setLevel(Level level);

LOG_LIB_API Level getLevel();

LOG_LIB_API void flush();

LOG_LIB_API bool shouldLog(Level level);

} // namespace log_lib
