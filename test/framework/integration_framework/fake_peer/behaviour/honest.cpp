/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/honest.hpp"

namespace integration_framework {

  void HonestFakePeerBehaviour::processYacMessage(
      const FakePeer::YacMessagePtr &message) {
    getFakePeer().voteForTheSame(message);
  }

  std::string HonestFakePeerBehaviour::getName() {
    return "honest behaviour";
  }

}  // namespace integration_framework
