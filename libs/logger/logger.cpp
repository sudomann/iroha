/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger/logger.hpp"

#include <mutex>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

static const std::string kDefaultPattern = R"([%Y-%m-%d %H:%M:%S.%F][%L] %n: %v)";
static const std::string kTagHierarchySeparator = "/";

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

  LoggerConfigTreeNode::LoggerConfigTreeNode(std::string tag,
                                             LoggerConfig config)
      : tag_(std::move(tag)), config_(std::move(config)){};

  LoggerConfigTreeNodePtr LoggerConfigTreeNode::addChild(
      std::string tag,
      boost::optional<LogLevel> log_level,
      boost::optional<LogPatterns> patterns) {
    LoggerConfig child_config{
        log_level.value_or(config_.log_level),
        patterns ? LogPatterns{std::move(*patterns)} : config_.patterns};
    auto child =
        std::make_shared<LoggerConfigTreeNode>(tag, std::move(child_config));
    auto map_elem = std::make_pair<const std::string, LoggerConfigTreeNodePtr>(
        std::move(tag), std::move(child));
    return children_.emplace(std::move(map_elem)).first->second;
  }

  const std::string &LoggerConfigTreeNode::getTag() const {
    return tag_;
  }

  const LoggerConfig &LoggerConfigTreeNode::getConfig() const {
    return config_;
  }

  boost::optional<LoggerConfigTreeNodePtr> LoggerConfigTreeNode::getChild(
      const std::string &tag) {
    const auto it = children_.find(tag);
    if (it == children_.end()) {
      return boost::none;
    }
    return it->second;
  }

  boost::optional<ConstLoggerConfigTreeNodePtr> LoggerConfigTreeNode::getChild(
      const std::string &tag) const {
    const auto it = children_.find(tag);
    if (it == children_.end()) {
      return boost::none;
    }
    return std::static_pointer_cast<const LoggerConfigTreeNode>(it->second);
  }

  class Logger::Impl {
   public:
    virtual ~Impl() = default;

    /// Constructor.
    ///
    /// @param tag - the tag of constructed logger
    /// @param tree - the logger tree configuration
    /// @param spdlog_logger - the spdlog logger to use
    Impl(std::string tag, ConstLoggerConfigTreeNodePtr tree)
        : tag_(std::move(tag)),
          tree_(std::move(tree)),
          logger_(spdlog::stdout_color_mt(tag)) {
      setupLogger();
    }

    LoggerPtr getChild(std::string tag) {
      std::lock_guard<std::mutex> lock(children_mutex_);
      const auto child_it = children_.find(tag);
      if (child_it != children_.end()) {
        return child_it->second;
      }
      // if a node for this child is not found in the tree config, create a
      // new standalone logger using this logger's settings
      auto child_config = tree_->getChild(tag).value_or(
          std::make_shared<LoggerConfigTreeNode>(tag, tree_->getConfig()));
      Logger new_child(tag_ + kTagHierarchySeparator + tag,
                       std::move(child_config));
      return children_
          .emplace(std::make_pair(tag, std::make_shared<Logger>(new_child)))
          .first->second;
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
      const auto it = kSpdLogLevels.find(tree_->getConfig().log_level);
      BOOST_ASSERT_MSG(it != kSpdLogLevels.end(), "Unknown log level!");
      logger_->set_level(it == kSpdLogLevels.end()
                             ? kSpdLogLevels.at(kDefaultLogLevel)
                             : it->second);
      logger_->set_pattern(
          kDefaultLogPatterns.getPattern(tree_->getConfig().log_level));
    }

    // --- Fields accessible by logger::Logger ---
   public:
    const std::string tag_;
    const ConstLoggerConfigTreeNodePtr tree_;
    const std::shared_ptr<spdlog::logger> logger_;

    // --- Internal use fields ---
   private:
    std::unordered_map<std::string, LoggerPtr> children_;
    std::mutex children_mutex_;
  };

  Logger::Logger(ConstLoggerConfigTreeNodePtr tree_config)
      : impl_(std::make_shared<Impl>(tree_config->getTag(), tree_config)) {}

  Logger::Logger(std::string tag, LoggerConfig config)
      : impl_(std::make_shared<Impl>(
            tag,
            std::make_shared<LoggerConfigTreeNode>(tag, std::move(config)))) {}

  Logger::Logger(std::string tag, ConstLoggerConfigTreeNodePtr tree_config)
      : impl_(std::make_shared<Impl>(std::move(tag), std::move(tree_config))) {}

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
    return impl_->tree_->getConfig().log_level <= level;
  }

  LoggerPtr Logger::getChild(std::string tag) {
    return impl_->getChild(tag);
  }

  std::string boolRepr(bool value) {
    return value ? "true" : "false";
  }

}  // namespace logger
