/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_LOGGER_SPDLOG_HPP
#define IROHA_LOGGER_SPDLOG_HPP

#include "logger/logger.hpp"

#include <map>
#include <memory>
#include <string>

#include <boost/optional.hpp>

namespace spdlog {
  class logger;
}

namespace logger {

  class LogPatterns;
  struct LoggerConfig;

  using ConstLoggerConfigPtr = std::shared_ptr<const LoggerConfig>;

  extern const LogPatterns kDefaultLogPatterns;

  /// Patterns for logging depending on the log level.
  class LogPatterns {
   public:
    /// Set a logging pattern for the given level.
    void setPattern(LogLevel level, std::string pattern);

    /**
     * Get the logging pattern for the given level. If not set, get the
     * next present more verbose level pattern, if any, or the default
     * pattern.
     */
    std::string getPattern(LogLevel level) const;

   private:
    std::map<LogLevel, std::string> patterns_;
  };

  // TODO mboldyrev 29.12.2018 IR-188 Add sink options (console, file, syslog)
  struct LoggerConfig {
    LogLevel log_level;
    LogPatterns patterns;
  };

  class LoggerSpdlog : public Logger {
   public:
    using Level = LogLevel;

    /**
     * @param tag - the tag for logging (aka logger name)
     * @param config - logger configuration
     */
    LoggerSpdlog(std::string tag, ConstLoggerConfigPtr config);

    virtual ~LoggerSpdlog();

    // --- Logging functions ---

    template <typename... Args>
    void trace(const std::string &format, const Args &... args) const {
      log(LogLevel::kTrace, format, args...);
    }

    template <typename... Args>
    void debug(const std::string &format, const Args &... args) const {
      log(LogLevel::kDebug, format, args...);
    }

    template <typename... Args>
    void info(const std::string &format, const Args &... args) const {
      log(LogLevel::kInfo, format, args...);
    }

    template <typename... Args>
    void warn(const std::string &format, const Args &... args) const {
      log(LogLevel::kWarn, format, args...);
    }

    template <typename... Args>
    void error(const std::string &format, const Args &... args) const {
      log(LogLevel::kError, format, args...);
    }

    template <typename... Args>
    void critical(const std::string &format, const Args &... args) const {
      log(LogLevel::kCritical, format, args...);
    }

    template <typename... Args>
    void log(Level level,
             const std::string &format,
             const Args &... args) const {
      if (shouldLog(level)) {
        logInternal(level, format);  // TODO perform the actual formatting here
      }
    }

   private:
    void logInternal(Level level, const std::string &s) const override;

    /// Whether the configured logging level is at least as verbose as the
    /// one given in parameter.
    bool shouldLog(Level level) const override;

    /// Set Spdlog logger level and pattern.
    void setupLogger();

    const std::string tag_;
    const ConstLoggerConfigPtr config_;
    const std::shared_ptr<spdlog::logger> logger_;
  };

}  // namespace logger

#endif
