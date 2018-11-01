/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HONEST_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HONEST_HPP_

#include "framework/integration_framework/fake_peer/behaviour/empty.hpp"

namespace integration_framework {

  class HonestFakePeerBehaviour : public EmptyFakePeerBehaviour {
   public:
    virtual ~HonestFakePeerBehaviour() = default;

    void processYacMessage(const FakePeer::YacMessagePtr &message) override;

    std::string getName() override;
  };

}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HONEST_HPP_ */
