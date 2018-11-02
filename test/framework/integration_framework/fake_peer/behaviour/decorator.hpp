/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_

#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"

namespace integration_framework {
  namespace fake_peer {

    class BehaviourDecorator : public Behaviour {
     public:
      BehaviourDecorator(Behaviour &base_behaviour);

      ~BehaviourDecorator() = default;

      void processMstMessage(const FakePeer::MstMessagePtr &message) override =
          0;
      void processYacMessage(const FakePeer::YacMessagePtr &message) override =
          0;
      void processOsBatch(const FakePeer::OsBatchPtr &batch) override = 0;
      void processOgProposal(const FakePeer::OgProposalPtr &proposal) override =
          0;

      std::string getName() override = 0;

     protected:
      Behaviour &base_behaviour_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_ */
