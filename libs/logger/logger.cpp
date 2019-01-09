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
    Impl(std::string tag,
         ConstLoggerConfigTreeNodePtr tree,
         std::shared_ptr<spdlog::logger> spdlog_logger)
        : tag_(std::move(tag)),
          tree_(std::move(tree)),
          logger_(std::move(spdlog_logger)) {
      setupLogger();
    }

    virtual LoggerPtr getChild(std::string tag, LoggerThreadSafety ts) = 0;

   protected:
    LoggerPtr getChildSingleTread(std::string tag, LoggerThreadSafety ts) {
      std::string child_full_tag = tag_ + kTagHierarchySeparator + tag;
      auto &children_by_tag = children_[tag];
      const auto child_it = children_by_tag.find(ts);
      if (child_it != children_by_tag.end()) {
        return child_it->second;
      }
      // if a node for this child is not found in the tree config, create a
      // new standalone logger using this logger's settings
      auto child_config = tree_->getChild(tag).value_or(
          std::make_shared<LoggerConfigTreeNode>(tag, tree_->getConfig()));
      Logger new_child(
          tag_ + kTagHierarchySeparator + tag, std::move(child_config), ts);
      return children_by_tag
          .emplace(std::make_pair(ts, std::make_shared<Logger>(new_child)))
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
    std::unordered_map<std::string, std::map<LoggerThreadSafety, LoggerPtr>>
        children_;
  };

  class LoggerImplSingleThread : public Logger::Impl {
   public:
    template <typename... Types>
    LoggerImplSingleThread(std::string tag, Types &&... args)
        : Logger::Impl(
              tag, std::forward<Types>(args)..., spdlog::stdout_color_st(tag)) {
    }

    virtual ~LoggerImplSingleThread() = default;

    LoggerPtr getChild(std::string tag, LoggerThreadSafety ts) {
      return getChildSingleTread(tag, ts);
    }
  };

  class LoggerImplMultiThread : public Logger::Impl {
   public:
    template <typename... Types>
    LoggerImplMultiThread(std::string tag, Types &&... args)
        : Logger::Impl(
              tag, std::forward<Types>(args)..., spdlog::stdout_color_mt(tag)) {
    }

    virtual ~LoggerImplMultiThread() = default;

    LoggerPtr getChild(std::string tag, LoggerThreadSafety ts) {
      std::lock_guard<std::mutex> lock(children_mutex);
      return getChildSingleTread(tag, ts);
    }

   private:
    std::mutex children_mutex;
  };

  static std::unique_ptr<Logger::Impl> makeLoggerImpl(
      std::string tag,
      ConstLoggerConfigTreeNodePtr tree_config,
      LoggerThreadSafety ts) {
    switch (ts) {
      case LoggerThreadSafety::kSingleThread:
        return std::make_unique<LoggerImplSingleThread>(std::move(tag),
                                                        std::move(tree_config));
      case LoggerThreadSafety::kMultiThread:
        return std::make_unique<LoggerImplMultiThread>(std::move(tag),
                                                       std::move(tree_config));
      default:
        BOOST_THROW_EXCEPTION(std::runtime_error("Unknown logger type!"));
    }
  }

  Logger::Logger(ConstLoggerConfigTreeNodePtr tree_config, LoggerThreadSafety ts)
      : impl_(makeLoggerImpl(tree_config->getTag(), tree_config, ts)) {}

  Logger::Logger(std::string tag, LoggerConfig config, LoggerThreadSafety ts)
      : impl_(makeLoggerImpl(
            tag,
            std::make_shared<LoggerConfigTreeNode>(tag, std::move(config)),
            ts)) {}

  Logger::Logger(std::string tag,
                 ConstLoggerConfigTreeNodePtr tree_config,
                 LoggerThreadSafety ts)
      : impl_(makeLoggerImpl(std::move(tag), std::move(tree_config), ts)) {}

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

  LoggerPtr Logger::getChild(std::string tag, LoggerThreadSafety ts) {
    return impl_->getChild(tag, ts);
  }

  std::string boolRepr(bool value) {
    return value ? "true" : "false";
  }

}  // namespace logger
