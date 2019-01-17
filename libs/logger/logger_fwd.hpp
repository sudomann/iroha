/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP
#define IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP

#include <memory>

namespace logger {

  class Logger;
  class LogPatterns;
  class LoggerConfig;
  enum class LogLevel;

  using LoggerPtr = std::shared_ptr<Logger>;
  using ConstLoggerConfigPtr = std::shared_ptr<const LoggerConfig>;

}  // namespace logger

#endif // IROHA_SPDLOG_LOGGER_LOGGER_FWD_HPP
