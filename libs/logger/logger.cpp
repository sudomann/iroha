/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger/logger.hpp"

#include <mutex>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

static const std::string kDefaultPattern = R"([%Y-%m-%d %H:%M:%S.%F][%L] %n: %v)";

namespace logger {

  const LogLevel kDefaultLogLevel = LogLevel::kInfo;
  const LogPatterns kDefaultLogPatterns = ([] {
    LogPatterns p;
    p.setPattern(LogLevel::kTrace,
                 R"([%Y-%m-%d %H:%M:%S.%F][th:%t][%L] %n: %v)");
    p.setPattern(LogLevel::kInfo, kDefaultPattern);
    return p;
  })();

  void LogPatterns::setPattern(LogLevel level, std::string pattern) {
    patterns_[level] = pattern;
  }

  std::string LogPatterns::getPattern(LogLevel level) const {
    for (auto it = patterns_.rbegin(); it != patterns_.rend(); ++it) {
      if (it->first <= level) {
        return it->second;
      }
    }
    return kDefaultPattern;
  }

  class Logger::Impl {
   public:
    virtual ~Impl() = default;

    /// @param tag - the tag for logging (aka logger name)
    /// @param config - logger configuration
    Impl(std::string tag, ConstLoggerConfigPtr config)
        : tag_(std::move(tag)),
          config_(std::move(config)),
          logger_(spdlog::stdout_color_mt(tag)) {
      setupLogger();
    }

   private:
    void setupLogger() {
      static const std::map<LogLevel, const spdlog::level::level_enum>
          kSpdLogLevels = {{LogLevel::kTrace, spdlog::level::trace},
                           {LogLevel::kDebug, spdlog::level::debug},
                           {LogLevel::kInfo, spdlog::level::info},
                           {LogLevel::kWarn, spdlog::level::warn},
                           {LogLevel::kError, spdlog::level::err},
                           {LogLevel::kCritical, spdlog::level::critical}};
      const auto it = kSpdLogLevels.find(config_->log_level);
      BOOST_ASSERT_MSG(it != kSpdLogLevels.end(), "Unknown log level!");
      logger_->set_level(it == kSpdLogLevels.end()
                             ? kSpdLogLevels.at(kDefaultLogLevel)
                             : it->second);
      logger_->set_pattern(kDefaultLogPatterns.getPattern(config_->log_level));
    }

    // --- Fields accessible by logger::Logger ---
   public:
    const std::string tag_;
    const ConstLoggerConfigPtr config_;
    const std::shared_ptr<spdlog::logger> logger_;
  };

  Logger::Logger(std::string tag, ConstLoggerConfigPtr config)
      : impl_(std::make_shared<Impl>(tag, std::move(config))) {}

  Logger::Logger(const Logger &other) : impl_(other.impl_) {}

  Logger::Logger(Logger &&other) : impl_(std::move(other.impl_)) {}

  Logger &Logger::operator=(const Logger &other) {
    impl_ = other.impl_;
    return *this;
  }

  Logger &Logger::operator=(Logger &&other) {
    impl_ = std::move(other.impl_);
    return *this;
  }

  Logger::~Logger() = default;

  void Logger::logInternal(Level level, const std::string &s) const {
    switch (level) {
      case LogLevel::kTrace:
        impl_->logger_->trace(s);
        break;
      case LogLevel::kDebug:
        impl_->logger_->debug(s);
        break;
      case LogLevel::kInfo:
        impl_->logger_->info(s);
        break;
      case LogLevel::kWarn:
        impl_->logger_->warn(s);
        break;
      case LogLevel::kError:
        impl_->logger_->error(s);
        break;
      case LogLevel::kCritical:
        impl_->logger_->critical(s);
        break;
      default:
        BOOST_THROW_EXCEPTION(std::runtime_error("Unknown log level!"));
    }
  }

  bool Logger::shouldLog(Level level) const {
    return impl_->config_->log_level <= level;
  }

  std::string boolRepr(bool value) {
    return value ? "true" : "false";
  }

}  // namespace logger
