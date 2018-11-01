/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DELAYED_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DELAYED_HPP_

#include <chrono>

#include "framework/integration_framework/fake_peer/behaviour/decorator.hpp"

namespace integration_framework {

  class DelayedFakePeerBehaviour : public FakePeerBehaviourDecorator {
   public:
    DelayedFakePeerBehaviour(FakePeerBehaviour &base_behaviour,
                             std::chrono::milliseconds delay);

    ~DelayedFakePeerBehaviour() = default;

    void processMstMessage(const FakePeer::MstMessagePtr &message) override;
    void processYacMessage(const FakePeer::YacMessagePtr &message) override;
    void processOsBatch(const FakePeer::OsBatchPtr &batch) override;
    void processOgProposal(const FakePeer::OgProposalPtr &proposal) override;

    std::string getName() override;

   private:
    std::chrono::milliseconds delay_;
  };

}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DELAYED_HPP_ */
