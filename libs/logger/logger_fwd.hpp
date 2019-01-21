/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP
#define IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP

#include <memory>

/* It is preferable to include this header in files that do not contain
 * dereferencing of LoggerPtr and do not use the Logger class functions, because
 * the actual Logger class definition contains template member functions that
 * use template library functions, thus making the preprocessed source file much
 * bigger.
 */

namespace logger {

  class Logger;
  class LogPatterns;
  struct LoggerConfig;
  enum class LogLevel;

  using LoggerPtr = std::shared_ptr<Logger>;
  using ConstLoggerConfigPtr = std::shared_ptr<const LoggerConfig>;

}  // namespace logger

#endif // IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP
