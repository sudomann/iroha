/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_

#include <memory>
#include <vector>

#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"
#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "logger/logger.hpp"

namespace integration_framework {
  namespace fake_peer {

    class EmptyBehaviour : public Behaviour {
     public:
      virtual ~EmptyBehaviour() = default;

      void processMstMessage(const FakePeer::MstMessagePtr &message) override;
      void processYacMessage(const FakePeer::YacMessagePtr &message) override;
      void processOsBatch(const FakePeer::OsBatchPtr &batch) override;
      void processOgProposal(const FakePeer::OgProposalPtr &proposal) override;
      LoaderBlockRequestResult processLoaderBlockRequest(
          const FakePeer::LoaderBlockRequest &request) override;
      LoaderBlocksRequestResult processLoaderBlocksRequest(
          const FakePeer::LoaderBlocksRequest &request) override;

      virtual std::string getName();
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_ */
