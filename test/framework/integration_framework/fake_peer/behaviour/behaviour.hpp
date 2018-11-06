/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "logger/logger.hpp"

namespace shared_model {
  namespace proto {
    class Block;
  }
}  // namespace shared_model

namespace integration_framework {
  namespace fake_peer {

    class Behaviour {
     public:
      using LoaderBlockRequestResult =
          boost::optional<const shared_model::proto::Block &>;
      using LoaderBlocksRequestResult =
          std::vector<std::reference_wrapper<const shared_model::proto::Block>>;

      virtual ~Behaviour();

      /// Enable the behaviour for the given peer
      void adopt(const std::shared_ptr<FakePeer> &fake_peer);

      /// Disable the behaviour
      void absolve();

      virtual void processMstMessage(
          const FakePeer::MstMessagePtr &message) = 0;
      virtual void processYacMessage(
          const FakePeer::YacMessagePtr &message) = 0;
      virtual void processOsBatch(const FakePeer::OsBatchPtr &batch) = 0;
      virtual void processOgProposal(
          const FakePeer::OgProposalPtr &proposal) = 0;
      virtual LoaderBlockRequestResult processLoaderBlockRequest(
          const FakePeer::LoaderBlockRequest &request) = 0;
      virtual LoaderBlocksRequestResult processLoaderBlocksRequest(
          const FakePeer::LoaderBlocksRequest &request) = 0;

      virtual std::string getName() = 0;

     protected:
      FakePeer &getFakePeer();
      logger::Logger &getLogger();

     private:
      std::weak_ptr<FakePeer> fake_peer_wptr_;
      std::vector<rxcpp::subscription> subscriptions_;
      logger::Logger log_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_ */
