#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(logger STATIC logger.cpp)
target_link_libraries(logger
    spdlog
)

add_library(logger_manager STATIC logger_manager.cpp)
target_link_libraries(logger_manager
    logger
)
